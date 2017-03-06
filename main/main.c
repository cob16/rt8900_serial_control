//
// Created by cormac on 16/02/17.
//
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <argp.h>
#include <serial.h>

#include "serial.c"
#include "control_packet.c"

//reference taken from https://www.gnu.org/software/libc/manual/html_node/Argp-Example-3.htmlf
const char *argp_program_version = "0.0.1";
const char *argp_program_bug_address = "<cormac.brady@hotmai.co.uk>";
static char rt8900_doc[] = "Provides serial control for the YAESU FT-8900R Transceiver.";
static char rt8900_args_doc[] = "<serial port path>";

static struct argp_option rt8900options[] = {
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
static struct argp argp = { rt8900options, parse_opt, rt8900_args_doc, rt8900_doc };


///Creates the required packet to dial a number. they should then be added to the head of the queue*
void dial_number(SERIAL_CFG *cfg, int number) //TODO FINISH THIS
{
        //get the number of digits
        int num_digets = snprintf(NULL, 0, "%d", number);
        if (num_digets > 6){
                //this number
                printf("WARNING!: dialing a %d digit number! (%d) (Only 6 required for frequency inputs)", num_digets, number);
        }

        //create a char array of the digits
        char digits[num_digets];
        snprintf(digits, sizeof(num_digets), "%d", number);

        //add packets that 'press' the seletced buttons
        int i;
        for (i=0; i<num_digets; i++){
                create_button_packet(base_packet, button_from_int(digits[i] - '0'));

                send_new_packet(&c, packet);
        }

        return 0;
}


int main(int argc, char **argv)
{
        //Create our config
        SERIAL_CFG c;
        argp_parse (&argp, argc, argv, 0, 0, &c); //insert user options to config

        pthread_t packet_send_thread;

        pthread_barrier_t wait_for_init;
        pthread_barrier_init(&wait_for_init, NULL, 2);
        c.initialised = &wait_for_init;
        //cast our pointer pointer to void pointer for thread creation
        pthread_create(&packet_send_thread, NULL, send_control_packets, &c);

        pthread_barrier_wait(&wait_for_init); //wait for thread to be ready

        struct control_packet *new_packet = malloc(sizeof(*new_packet));
        memcpy(new_packet, &control_packet_defaults,sizeof(*new_packet));
        new_packet->squelch_left.section.data = 0x00;
        set_button(new_packet, &BUTTON_1);
        send_new_packet(&c, new_packet);

        struct control_packet *more_new_packet = malloc(sizeof(struct control_packet));
        memcpy(more_new_packet, &control_packet_defaults,sizeof(struct control_packet));
        more_new_packet->keypad_input_column.section.data = 0x00;
        send_new_packet(&c, more_new_packet);

        usleep(1000 * 1000);

        c.keep_alive = false;
        pthread_barrier_destroy(&wait_for_init);
        pthread_join(packet_send_thread, NULL);
        return 0;
}