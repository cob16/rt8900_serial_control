//
// Created by cormac on 16/02/17.
//
#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <argp.h>
#include <pthread.h>
#include <signal.h>

#include <unistd.h>
#include <serial.h>
#include <packet.h>

#include "control_packet.c"
#include "display_packet.c"

//reference taken from https://www.gnu.org/software/libc/manual/html_node/Argp-Example-3.htmlf
const char *argp_program_version = "0.0.1";
const char *argp_program_bug_address = "<cormac.brady@hotmai.co.uk>";
static char rt8900_doc[] = "Provides serial control for the YAESU FT-8900R Transceiver.";
static char rt8900_args_doc[] = "<serial port path>";

static struct argp_option rt8900options[] = {
        {"verbose", 'v', "LEVEL", OPTION_ARG_OPTIONAL,
                "Produce verbose output add a number to select level (1 = ERROR, 2= WARNING, 3=INFO, 4=ERROR, 5=DEBUG) output default is 'warning'."},
        {"hard-emulation", 991, 0, OPTION_ARG_OPTIONAL,
                "Exactly emulates the radio head instead of being lazy_sending (worse performance, no observed benefit, only useful for debugging)"},
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
                cfg->send.lazy_sending = arg ? true : false;
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

static SERIAL_CFG *g_conf = NULL;

///shutdown the program gracefully
void graceful_shutdown(int signal)
{
        //use strsignal(signal) to get the name from int if we want to add others latter
        log_msg(RT8900_INFO, "\nShutting down...\n");
        if (g_conf != NULL) {
                g_conf->send.keep_alive = false;
                if (g_conf->serial_fd != 0) {
                        tcflush(g_conf->serial_fd, TCIOFLUSH); //flush in/out buffers
                        close(g_conf->serial_fd);
                }
                fclose(stdin); //we close std so that user prompt (get_char) will stop blocking
        }
}

void init_graceful_shutdown(SERIAL_CFG *c)
{
        g_conf = c;
        signal(SIGINT, graceful_shutdown);
}

#define PROMPT_BUFFER_SIZE 32


char *read_prompt_line(void)
{
        // inspired by https://github.com/brenns10/lsh/blob/407938170e8b40d231781576e05282a41634848c/src/main.c
        int buffer_size = PROMPT_BUFFER_SIZE;
        char *buffer = malloc(sizeof(char) * buffer_size);
        if (!buffer) {
                log_msg(RT8900_ERROR, "Failed to allocate buffer");
                exit(EXIT_FAILURE);
        }

        int i;
        for (i = 0; ; i++) {

                if (i >= buffer_size) {
                        buffer_size = buffer_size * 2;
                        buffer = realloc(buffer, buffer_size);
                        if (!buffer) {
                                log_msg(RT8900_ERROR, "Failed to increase buffer size");
                                exit(EXIT_FAILURE);
                        }
                }

                int ch = getchar();

                if (ch == EOF || ch == '\n') {
                        buffer[i] = '\0';
                        return buffer;
                } else {
                        buffer[i] = ch;
                }
        }
}


/// Split string into arg array
char **split_line_args(char *line)
{
        // inspired by https://github.com/brenns10/lsh/blob/407938170e8b40d231781576e05282a41634848c/src/main.c
        int buffer_size = PROMPT_BUFFER_SIZE * 2;
        char **args = malloc(buffer_size * sizeof(char*));
        char *token;
        int i;

        token = strtok(line, " ");
        for (i = 0; (token != NULL || i < buffer_size) ; i++) {
                args[i] = token;
                token = strtok(NULL, " ");
        }
        args[i] = NULL;
        return args;
}

#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void print_invalid_command()
{
        printf("%s%s%s\n", ANSI_COLOR_YELLOW, "Invalid command", ANSI_COLOR_RESET);
}

int run_command(char **cmd, SERIAL_CFG *config, struct control_packet *base_packet)
{
        int num_args = 0;
        for (num_args = 0; cmd[num_args] != NULL; num_args++);

        int left_op;
        int right_op;

        switch (num_args){
        case 0:
                print_invalid_command();
                return 1;
        case 1:
                if (strcmp(cmd[0], "exit") == 0) {
                        printf("%sExiting%s\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET);
                        return 0;
                } else if (strcmp(cmd[0], "b") == 0){
                        struct radio_state *current_state = malloc(sizeof(*current_state));
                        struct display_packet packet;
                        if (get_display_packet(config, &packet) == 0) {
                                read_state_from_packet(&packet, current_state);

                                char* lbusy = (current_state->left.busy ? "Busy" : "Not Busy");
                                char* lmain = (is_main(current_state, &(current_state->left)) ? " - Main" : "");

                                char* rbusy = (current_state->right.busy ? "Busy" : "Not Busy");
                                char* rmain = (is_main(current_state, &(current_state->right)) ? " - Main" : "");

                                printf("Left  radio -> %s%s\nRight radio -> %s%s\n", lbusy, lmain, rbusy, rmain);
                        }
                        free(current_state);
                        current_state = NULL;
                } else {
                        print_invalid_command();
                }
                return 1;
        case 2:
                if (strcmp(cmd[0], "F") == 0){
                        left_op = (int)strtoimax(cmd[1], NULL, 10);
                        printf("%s Setting frequency -> %d %s\n", ANSI_COLOR_GREEN, left_op, ANSI_COLOR_RESET);
                        set_frequency(config, base_packet, left_op);
                } else {
                        print_invalid_command();
                }
                return 1;
        case 3:
                left_op = (int)strtoimax(cmd[1], NULL, 10);
                right_op = (int)strtoimax(cmd[2], NULL, 10);
                if (strcmp(cmd[0], "v") == 0) {
                        printf("%s Setting volume -> %d %d%s\n", ANSI_COLOR_GREEN, left_op, right_op, ANSI_COLOR_RESET);
                        set_volume(base_packet, left_op, right_op);

                } else if (strcmp(cmd[0], "s") == 0){
                        printf("%s Setting squelsh -> %d %d%s\n", ANSI_COLOR_GREEN, left_op, right_op, ANSI_COLOR_RESET);
                        set_squelch(base_packet, left_op, right_op);
                } else {
                        print_invalid_command();
                }
        default:
                return 1;
        }
}

void user_prompt(SERIAL_CFG *config, struct control_packet *base_packet)
{
        char *line;
        char **command_arr;

        while (config->send.keep_alive) {
                printf("> ");
                line  = read_prompt_line();;
                command_arr = split_line_args(line);

                int dont_exit = run_command(command_arr, config, base_packet);

                free(line);
                free(command_arr);
                if (!dont_exit){
                        break;
                }
        }
}

int main(int argc, char **argv)
{
        //Create our config
        SERIAL_CFG c = {
                .send.lazy_sending = true
        };
        argp_parse (&argp, argc, argv, 0, 0, &c); //insert user options to config

        //Create a thread to send oud control packets
        pthread_t packet_sender_thread;
        pthread_barrier_t wait_for_init;

        pthread_barrier_init(&wait_for_init, NULL, 2);
        c.send.initialised = &wait_for_init;

        pthread_create(&packet_sender_thread, NULL, send_control_packets, &c);
        pthread_barrier_wait(&wait_for_init); //wait for thread to be ready
        init_graceful_shutdown(&c);

        //read out the current state of the hardware
//        struct display_packet *current_state = malloc(sizeof(struct display_packet));
//        get_display_packet(&c, current_state);

        //todo check if any existing state needs to be transferred to our starting packet

        //Setup our initial packet that will be sent
        maloc_control_packet(start_packet)
        memcpy(start_packet, &control_packet_defaults ,sizeof(*start_packet));
        send_new_packet(&c, start_packet, PACKET_ONLY_SEND);

        //todo block until display packets are revived
        //todo add a block until radio is able to revived commands (boot time)

        user_prompt(&c, start_packet);

        graceful_shutdown(0);
         //This should be false already, just in case we will make shure
        pthread_barrier_destroy(&wait_for_init);
        pthread_join(packet_sender_thread, NULL);
        return 0;
}