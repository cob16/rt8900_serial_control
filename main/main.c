//
// Created by cormac on 16/02/17.
//
#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <argp.h>

#include <unistd.h>

#include "control_packet.c"

//reference taken from https://www.gnu.org/software/libc/manual/html_node/Argp-Example-3.htmlf
const char *argp_program_version = "0.0.1";
const char *argp_program_bug_address = "<cormac.brady@hotmai.co.uk>";
static char rt8900_doc[] = "Provides serial control for the YAESU FT-8900R Transceiver.";
static char rt8900_args_doc[] = "<serial port path>";

static struct argp_option rt8900options[] = {
        {"verbose",'v',0,0, "Produce verbose output" },
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
        case 'v':
                cfg->verbose = true;
                break;
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

struct control_packet * create_a_packet(void) {
        struct control_packet *packet = malloc(sizeof(*packet));
        memcpy(packet, &control_packet_defaults,sizeof(*packet));
        set_button(packet, &BUTTON_1);
        return (packet);
}

int main(int argc, char **argv)
{
        //Create our config
        SERIAL_CFG c = {
                .verbose = false,
                .lazy = true
        };
        argp_parse (&argp, argc, argv, 0, 0, &c); //insert user options to config

        pthread_t packet_send_thread;

        pthread_barrier_t wait_for_init;
        pthread_barrier_init(&wait_for_init, NULL, 2);
        c.initialised = &wait_for_init;
        //cast our pointer pointer to void pointer for thread creation
        pthread_create(&packet_send_thread, NULL, send_control_packets, &c);
        pthread_barrier_wait(&wait_for_init); //wait for thread to be ready

        create_packet(start_packet)
        memcpy(start_packet, &control_packet_defaults ,sizeof(*start_packet));

        start_packet->squelch_left.section.data = DATA_MAX_NUM;
        start_packet->squelch_right.section.data = DATA_MAX_NUM; //max is mim

        start_packet->volume_control_left.section.data = DATA_MIN_NUM;
        start_packet->volume_control_right.section.data = DATA_MIN_NUM;

        send_new_packet(&c, start_packet, 1);

        sleep(5);

        set_frequency(&c, start_packet, 145501);

//        c.keep_alive = false;
        pthread_barrier_destroy(&wait_for_init);
        pthread_join(packet_send_thread, NULL);
        return 0;
}