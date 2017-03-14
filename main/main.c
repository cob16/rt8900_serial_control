//
// Created by cormac on 16/02/17.
//
#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include <pthread.h>
#include <signal.h>

#include <unistd.h>
#include <serial.h>

#include "control_packet.c"

//reference taken from https://www.gnu.org/software/libc/manual/html_node/Argp-Example-3.htmlf
const char *argp_program_version = "0.0.1";
const char *argp_program_bug_address = "<cormac.brady@hotmai.co.uk>";
static char rt8900_doc[] = "Provides serial control for the YAESU FT-8900R Transceiver.";
static char rt8900_args_doc[] = "<serial port path>";

static struct argp_option rt8900options[] = {
        {"verbose", 'v', "LEVEL", OPTION_ARG_OPTIONAL,
                "Produce verbose output add a number to select level (1 = ERROR, 2= WARNING, 3=INFO, 4=ERROR, 5=DEBUG) output default is 'warning'."},
        {"hard-emulation", 991, 0, OPTION_ARG_OPTIONAL,
                "Exactly emulates the radio head instead of being lazy (worse performance, no observed benefit, only useful for debugging)"},
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
        case 991:
                cfg->lazy = arg ? true : false;
                break;
        case 'v':
                rt8900_verbose = arg ? (enum rt8900_logging_level) atoi (arg) : RT8900_INFO;
                break;
        case ARGP_KEY_ARG:
                if (state->arg_num >= 1)
                        /* Too many arguments. */
                        argp_usage(state);

                cfg->serial_path = arg;
                break;

        case ARGP_KEY_END:
                if (state->arg_num < 1)
                        /* Not enough arguments. */
                        argp_usage(state);
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

static bool *keepalive = NULL;
void graceful_shutdown(int signal)
{
        //use strsignal(signal) to get the name from int if we want to add others latter
        log_msg(RT8900_INFO, "\nReceived SIGINT signal. Shutting down...\n");
        if (keepalive != NULL) {
                *keepalive = false;
        }
}


void init_graceful_shutdown(SERIAL_CFG *c)
{
        keepalive = &(c->keep_alive);
        signal(SIGINT, graceful_shutdown);
}

int main(int argc, char **argv)
{
        //Create our config
        SERIAL_CFG c = {
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
        init_graceful_shutdown(&c);

        maloc_control_packet(start_packet)
        memcpy(start_packet, &control_packet_defaults ,sizeof(*start_packet));


        set_squelch(start_packet, 127, 127);
        set_volume(start_packet, 20, 20);

        send_new_packet(&c, start_packet, PACKET_SEND_ONLY);

        sleep(5);

        set_frequency(&c, start_packet, 145501);

//        c.keep_alive = false;
        pthread_barrier_destroy(&wait_for_init);
        pthread_join(packet_send_thread, NULL);
        return 0;
}