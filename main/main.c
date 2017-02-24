//
// Created by cormac on 16/02/17.
//
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include <stdio.h>
#include <pthread.h>
#include <control_packet.h>

#include "control_packet.c"
#include "serial.c"

//refrence taken from https://www.gnu.org/software/libc/manual/html_node/Argp-Example-3.htmlf
const char *argp_program_version = "0.0.1";
const char *argp_program_bug_address = "<cormac.brady@hotmai.co.uk>";
static char doc[] = "Provides serial control for the YAESU FT-8900R Transceiver.";
static char args_doc[] = "<serial port path>";

static struct argp_option options[] = {
        { 0 }
};

///Parse options
static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
        /* Get the input argument from argp_parse, which we
           know is a pointer to our arguments structure. */
        SERIAL_CFG *cfg = state->input;

        switch (key)
        {
                case ARGP_KEY_ARG:
                        if (state->arg_num >= 1)
                                /* Too many arguments. */
                                argp_usage (state);

                        cfg->serial_path = arg;
                        break;

                case ARGP_KEY_END:
                        if (state->arg_num < 1)
                                /* Not enough arguments. */
                                argp_usage (state);
                        break;

                default:
                        return ARGP_ERR_UNKNOWN;
        }
        return 0;
}
static struct argp argp = { options, parse_opt, args_doc, doc };

void print_char(char byte)
{
        int i;
        for (i = 0; i < 8; i++) {
                printf("%d", ((byte<< i) & 0x80) != 0);
        }
        printf("\n");
}

void* start_send_packet(void* c)
{
        SERIAL_CFG *conf = (SERIAL_CFG*) c;
        CONTROL_PACKET** packet_ptr = conf->packet;
        CONTROL_PACKET* last_ptr;

        printf("--------------------------\n");
        printf("-- STARTING CONTROLL PACKET THREAD %p\n", packet_ptr);

        int packets_sent = 0; //TODO does it matter that this counter will overflow in 246 days?
        int packets_sent_to_user = 0;
        while (conf->keep_alive) {
                CONTROL_PACKET *packet = *packet_ptr;
                if (packet != last_ptr) {
                        CONTROL_PACKET_INDEXED packet_arr = {.as_struct = *packet};
                        int i;
                        printf("\n");
                        printf("--------------------------\n");
                        printf("SERIAL CONTROLL THREAD\nnew pointer address: %p \nNow sending:\n", packet);
                        for (i = 0; i < sizeof(packet_arr.as_array); i++) {
                                print_char(packet_arr.as_array[i].raw);
                        }
                        printf("--------------------------\n");
                        last_ptr = packet;
                        packets_sent = 0;
                }else{
                        packets_sent++;
                        if (packets_sent - packets_sent_to_user > 100){
                                printf("Sent: %3d Packets\r", packets_sent); /* \r returns the caret to the line start */
                                fflush(stdout);
                                packets_sent_to_user = packets_sent;
                        }
                }

                //TODO SEND THE actual PACKET

                #ifdef _WIN32
                        Sleep(MILLISECONDS_BETWEEN_PACKETS);
                #else
                        usleep(MILLISECONDS_BETWEEN_PACKETS * 1000);  /* sleep 3 milliSeconds */
                #endif
        }
        printf("\n");
        printf("-- ENDING CONTROL PACKET THREAD\n");
}

int main(int argc, char **argv)
{
        SERIAL_CFG c = {
                .keep_alive = true,
        };

        argp_parse (&argp, argc, argv, 0, 0, &c);

        pthread_t packet_send_thread;

        CONTROL_PACKET *packet = malloc(sizeof(CONTROL_PACKET));
        memcpy(packet,&control_packet_defaults,sizeof(CONTROL_PACKET));

        CONTROL_PACKET **packet_ptr_ptr = &packet;
        c.packet = packet_ptr_ptr;

        //cast our pointer pointer to void pointer for thred creation
        pthread_create(&packet_send_thread, NULL, start_send_packet, &c);

        usleep(1000 * 1000);

        CONTROL_PACKET *new_packet = malloc(sizeof(CONTROL_PACKET));
        memcpy(new_packet, &control_packet_defaults,sizeof(CONTROL_PACKET));
        new_packet->ptt.section.data = 0x00;

        send_new_packet(&c, new_packet);

        usleep(1000 * 1000);

        c.keep_alive = false;
        pthread_join(packet_send_thread, NULL);

        free(new_packet);
        return 0;
}