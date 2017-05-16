//
// Created by cormac on 16/02/17.
//
#include "main.h"

#include <inttypes.h>
#include <argp.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

//reference taken from https://www.gnu.org/software/libc/manual/html_node/Argp-Example-3.htmlf
const char *argp_program_version = "0.0.1";
const char *argp_program_bug_address = "<cormac.brady@hotmail.co.uk>";
static char rt8900_doc[] = "Provides serial control for the YAESU FT-8900R Transceiver.";
static char rt8900_args_doc[] = "<serial port path>";

static struct argp_option rt8900options[] = {
        {"dtr-on", 'd', 0, OPTION_ARG_OPTIONAL,
                "Use the DTR pin of the serial connection as a power button for the rig. (REQUIRES compatible hardware)"},
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
        switch (key) {
        case 991:
                cfg->send.lazy_sending = arg ? true : false;
                break;
        case 'v':
                set_log_level( (enum rt8900_logging_level) atoi(arg));
                break;
        case 'd':
                cfg->send.dtr_pin_for_on = true;
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

static SERIAL_CFG *g_conf = NULL;

///shutdown the program gracefully
void graceful_shutdown(int signal)
{
        //use strsignal(signal) to get the name from int if we want to add others latter
        log_msg(RT8900_INFO, "\nShutting down...\n");
        if (g_conf != NULL) {
                shutdown_threads(g_conf);
        }
        /*  We close stdin so that our shell user prompt's
         * get_char() call will stop blocking and return */
        fclose(stdin);
}

void init_graceful_shutdown(SERIAL_CFG *c)
{
        g_conf = c;
        signal(SIGINT, graceful_shutdown);
}

char *read_prompt_line(void)
{
        // inspired by https://github.com/brenns10/lsh/blob/407938170e8b40d231781576e05282a41634848c/src/main.c
        int buffer_size = PROMPT_BUFFER_SIZE;
        char *buffer = malloc(sizeof(char) * buffer_size);
        if (buffer == NULL) {
                log_msg(RT8900_FATAL, "Failed to allocate buffer");
                shutdown_threads(g_conf);
                exit(EXIT_FAILURE);
        }

        int i;
        for (i = 0; ; i++) {

                if (i >= buffer_size) {
                        buffer_size = buffer_size * 2;
                        buffer = realloc(buffer, buffer_size);
                        if (buffer == NULL) {
                                log_msg(RT8900_ERROR, "Failed to increase buffer size");
                                graceful_shutdown(1);
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
        char **args = malloc(buffer_size * sizeof(char *));
        if (args == NULL) {
                log_msg(RT8900_FATAL, "Failed to allocate buffer");
                graceful_shutdown(1);
                exit(EXIT_FAILURE);
        }
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

void print_invalid_command()
{
        printf("%s%s%s\n", ANSI_COLOR_YELLOW, "Invalid command! type help for a list of valid user_commands", ANSI_COLOR_RESET);
}

/* shell implementation inspired by (modified and added to my cormac brady)
 * https://github.com/brenns10/lsh/blob/407938170e8b40d231781576e05282a41634848c/src/main.c */

struct cmd{
    //pointer to function wiht this signature
    int (*cmd_pointer)(char **args, SERIAL_CFG *config, struct control_packet *base_packet);
    char *keyword;
    char *discription;
    char *usage;
    int num_args;
};
 /* first element the keyword to call the functoon
 * the second is the number of arguments required
 * */

struct cmd user_commands[] = {
        {&cmd_help, "help", "Prints this help text", 0, 0},
        {&cmd_exit, "exit" , "Shutdown the application", 0, 0},
        {&cmd_get_frequency, "f" , "Shows the currently set frequency's (down to the fist decimal place)", 0, 0},
        {&cmd_get_power, "p" , "Show current power levels (i.e 1 = low, 5 = high)", 0, 0},
        {&cmd_get_volume, "v" , "Show current speaker volumes", 0, 0},
        {&cmd_get_squelch, "s" , "Show current squelch levels (127 is fully open)", 0, 0},
        {&cmd_get_busy, "b" , "Shows if radios are reporting busy", 0, 0},
        {&cmd_get_main, "m" , "Shows which radio is main", 0, 0},
        {&cmd_get_ptt, "t" , "Shows if the radio is currently transiting", 0 , 0},

        {&cmd_set_frequency, "F" , "Sets the frequency of the currently main radio", "[kHz]", 1},
        {&cmd_set_main, "M" , "Sets the given side to be the main radio", "[l or r]", 1},
        {&cmd_set_ptt, "T" , "Sets on/off radio transmit", "[1 to transmit or 0 to stop]", 1},
        {&cmd_set_power, "P" , "Set the transmission power of both radios 1 == low, 2 == medium, 5 = high", "[left power] [right power]", 2},
        {&cmd_set_volume, "V" , "Set the radio speaker volumes", "[left vol] [right vol]", 2},
        {&cmd_set_squelch, "S" , "Set radio squelch levels volumes between 0 (full) and 127 (no squelch)", "[left sql] [right sql]", 2},
};


struct radio_state *getState(const SERIAL_CFG *config);

int num_of_avalable_commands()
{
        return sizeof(user_commands) / sizeof(struct cmd);
}

void print_invalid_arguments(int given, struct cmd *cmd)
{
        printf("%s Invalid number of arguments given (%d). '%s' expects %d arguments %s\n",
               ANSI_COLOR_YELLOW,
               given,
               cmd->keyword,
               cmd->num_args,
               ANSI_COLOR_RESET
        );
}

int run_cmd(SERIAL_CFG *config, struct control_packet *base_packet, char **args)
{

        if (args[0] == NULL) { //empty line was entered.
                return 1;
        }

        int num_args = 0;
        for (num_args = 0; args[num_args] != NULL; num_args++);

        int i;
        for (i = 0; i < num_of_avalable_commands(); i++) {
                //command matches
                if (strcmp(args[0], user_commands[i].keyword) == 0) {
                        //has the right number of args (+1 for the cmd name)
                        if ((user_commands[i].num_args + 1) == num_args) {

                                return (*(user_commands[i].cmd_pointer))(args, config, base_packet);
                        } else {
                                print_invalid_arguments((num_args - 1), &(user_commands[i]));
                                return 1;
                        }
                }
        }

        print_invalid_command();
        return 1; //invalid command
}

int cmd_help(char **args, SERIAL_CFG *config, struct control_packet *base_packet)
{
        puts("The following user_commands are available to use:");
        int i;
        for (i = 0; i < num_of_avalable_commands(); i++) {
                printf("  %s %s%s%s -- %s\n\n",
                       user_commands[i].keyword,
                       ANSI_COLOR_YELLOW,
                       user_commands[i].usage == NULL ?  "" : user_commands[i].usage,
                       ANSI_COLOR_RESET,
                       user_commands[i].discription
                );
        }

        return 1;
}

int cmd_exit(char **args, SERIAL_CFG *config, struct control_packet *base_packet)
{
        printf("%sShutting down.. %s\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET);
        return 0;
}

int cmd_get_frequency(char **args, SERIAL_CFG *config, struct control_packet *base_packet)
{
        struct radio_state *current_state = malloc(sizeof(*current_state));
        if (current_state == NULL) {
                log_msg(RT8900_FATAL, "get_frequency failed to allocate new memory");
                return 0;
        }
        DISPLAY_PACKET display_packet;
        if (get_display_packet(config, display_packet) == 1) {
                read_frequency(display_packet, current_state);
                printf("Left  Frequency -> %d\n", current_state->left.frequency);
                printf("Right Frequency -> %d\n", current_state->right.frequency);
        }

        free(current_state);
        return 1;
};

int cmd_set_frequency(char **args, SERIAL_CFG *config, struct control_packet *base_packet)
{
        int frequency = (int) strtoimax(args[1], NULL, 10);

        if (in_freq_range(frequency) != INVALID_FREQUENCY) {
                printf("%s Setting frequency -> %d %s\n", ANSI_COLOR_GREEN, frequency, ANSI_COLOR_RESET);
                set_frequency(config, base_packet, frequency);
        } else {
                printf("%s%d is not a valid frequency for this radio.%s\n", ANSI_COLOR_YELLOW, frequency, ANSI_COLOR_RESET);
        }
        return 1;
};

int cmd_get_busy(char **args, SERIAL_CFG *config, struct control_packet *base_packet)
{
        struct radio_state *current_state = malloc(sizeof(*current_state));
        if (current_state == NULL) {
                log_msg(RT8900_FATAL, "cmd_get_busy failed to allocate new memory");
                return 0;
        }
        DISPLAY_PACKET display_packet;
        get_display_packet(config, display_packet);

        read_busy(display_packet, current_state);

        char* lbusy = (current_state->left.busy ? "Busy" : "Not Busy");
        char* rbusy = (current_state->right.busy ? "Busy" : "Not Busy");

        printf("Left  radio -> %s\nRight radio -> %s\n", lbusy, rbusy);

        free(current_state);
        return 1;
};

int cmd_get_main(char **args, SERIAL_CFG *config, struct control_packet *base_packet)
{
        struct radio_state *current_state = malloc(sizeof(*current_state));
        if (current_state == NULL) {
                log_msg(RT8900_FATAL, "cmd_get_main failed to allocate new memory");
                return 0;
        }
        DISPLAY_PACKET display_packet;
        get_display_packet(config, display_packet);

        read_main(display_packet, current_state);

        char* lmain = (is_main(current_state, &(current_state->left)) ? "<- Main" : "");
        char* rmain = (is_main(current_state, &(current_state->right)) ? "<- Main" : "");

        printf("Left  radio %s\nRight radio %s\n", lmain, rmain);

        free(current_state);
        return 1;
};

int cmd_set_main(char **args, SERIAL_CFG *config, struct control_packet *base_packet)
{
        struct radio_state *current_state = malloc(sizeof(*current_state)); //freed by set_main_radio()
        if (current_state == NULL) {
                log_msg(RT8900_FATAL, "cmd_set_main failed to allocate new memory");
                return 0;
        }
        DISPLAY_PACKET display_packet;
        get_display_packet(config, display_packet);
        read_main(display_packet, current_state);

        if (strcmp(args[1], "l") == 0) {
                if (is_main(current_state, &(current_state->right))) {
                        set_main_radio(config, base_packet, RADIO_LEFT);
                }
                printf("%s Setting Main radio to -> left %s\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET);

        } else if (strcmp(args[1], "r") == 0){
                if (is_main(current_state, &(current_state->left))) {
                        set_main_radio(config, base_packet, RADIO_RIGHT);
                }
                printf("%s Main radio set to -> right %s\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET);
        } else {
                printf("%s use 'l' & 'r' to set left or right %s\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
        }

        free(current_state);
        return 1;
}

int cmd_get_power(char **args, SERIAL_CFG *config, struct control_packet *base_packet)
{
        struct radio_state *current_state = malloc(sizeof(*current_state));
        if (current_state == NULL) {
                log_msg(RT8900_FATAL, "cmd_get_power failed to allocate new memory");
                return 0;
        }
        DISPLAY_PACKET display_packet;
        get_display_packet(config, display_packet);

        read_power_fuzzy(display_packet, current_state);
        printf("Left  power -> %d\n", current_state->left.power_level);
        printf("Right power -> %d\n", current_state->right.power_level);

        free(current_state);
        return 1;
};

int cmd_get_ptt(char **args, SERIAL_CFG *config, struct control_packet *base_packet)
{
        printf("TRANSMITTING -> %s\n", (base_packet->ptt.section.data == 0)? "TRUE" : "FALSE");
        return 1;
}


int cmd_set_ptt(char **args, SERIAL_CFG *config, struct control_packet *base_packet)
{
        struct radio_state *current_state = malloc(sizeof(*current_state));
        if (current_state == NULL) {
                log_msg(RT8900_FATAL, "cmd_set_ptt failed to allocate new memory");
                return 0;
        }
        DISPLAY_PACKET display_packet;
        get_display_packet(config, display_packet);

        read_main(display_packet, current_state);
        read_frequency(display_packet, current_state);

        if (current_state->main == NULL) {
                log_msg(RT8900_ERROR, "Could not determine main radio");
        }

        int current_frequency = get_frequency(current_state->main);

        if (in_freq_range(current_frequency) == VALID_FREQUENCY) {
                ptt(base_packet, (int) strtoimax(args[1], NULL, 10));
        } else {
                printf("%sThe radio does not permit PTT at ths frequency %s\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
        }

        free(current_state);

        return 1;
}

int cmd_set_power(char **args, SERIAL_CFG *config, struct control_packet *base_packet)
{
        struct radio_state *current_state = malloc(sizeof(*current_state));
        if (current_state == NULL) {
                log_msg(RT8900_FATAL, "cmd_set_power failed to allocate new memory");
                return 0;
        }
        DISPLAY_PACKET display_packet;
        get_display_packet(config, display_packet);
        read_frequency(display_packet, current_state);

        switch (set_left_power_level(config, base_packet, (enum rt8900_power_level) (int) strtoimax(args[1], NULL, 10))) {
        case 0:
                printf("%s Set left power level %s\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET);
                break;
        case 1:
                printf("%s INVALID LEFT POWER LEVEL %s\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
                break;
        default:
                break;
        }

        switch (set_right_power_level(config, base_packet, (enum rt8900_power_level) (int) strtoimax(args[2], NULL, 10))) {
        case 0:
                printf("%s Set right power level %s\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET);
                break;
        case 1:
                printf("%s INVALID RIGHT POWER LEVEL %s\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
                break;
        default:
                break;
        }

        free(current_state);
        return 1;
}

int cmd_set_volume(char **args, SERIAL_CFG *config, struct control_packet *base_packet)
{
        int left_op = (int)strtoimax(args[1], NULL, 10);
        int right_op = (int)strtoimax(args[2], NULL, 10);
        printf("%s Setting volume -> %d %d%s\n", ANSI_COLOR_GREEN, left_op, right_op, ANSI_COLOR_RESET);
        set_volume(base_packet, left_op, right_op);

        return 1;
}

int cmd_get_volume(char **args, SERIAL_CFG *config, struct control_packet *base_packet)
{
        printf("%s Left volume  -> %d \n Right volume -> %d %s\n",
               ANSI_COLOR_GREEN,
               get_volume_left(base_packet),
               get_volume_right(base_packet),
               ANSI_COLOR_RESET
        );
        return 1;
}

int cmd_set_squelch(char **args, SERIAL_CFG *config, struct control_packet *base_packet)
{
        int left_op = (int)strtoimax(args[1], NULL, 10);
        int right_op = (int)strtoimax(args[2], NULL, 10);
        printf("%s Setting squelch -> %d %d%s\n", ANSI_COLOR_GREEN, left_op, right_op, ANSI_COLOR_RESET);
        set_squelch(base_packet, left_op, right_op);

        return 1;
}

int cmd_get_squelch(char **args, SERIAL_CFG *config, struct control_packet *base_packet)
{
        printf("%s Left squelch  -> %d \n Right squelch -> %d %s\n",
               ANSI_COLOR_GREEN,
               get_squelch_left(base_packet),
               get_squelch_right(base_packet),
               ANSI_COLOR_RESET
        );
        return 1;
}

void user_prompt(SERIAL_CFG *config, struct control_packet *base_packet)
{
        char *line;
        char **command_arr;

        while (config->send.keep_alive) {
                printf("> ");
                line  = read_prompt_line();;
                command_arr = split_line_args(line);

                int dont_exit = run_cmd(config, base_packet, command_arr);

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
                .shutdown_on_timeout = false,//todo make true
                .send.lazy_sending = true,
                .send.dtr_pin_for_on = false,
        };
        argp_parse (&argp, argc, argv, 0, 0, &c); //insert user options to config


        //Create a thread to send oud control packets
        pthread_t packet_sender_thread;
        pthread_t packet_receive_thread;
        pthread_barrier_t wait_for_sender;

        pthread_barrier_init(&wait_for_sender, NULL, 2);
        c.send.initialised = &wait_for_sender;

        init_graceful_shutdown(&c);

        pthread_create(&packet_sender_thread, NULL, send_control_packets, &c);
        pthread_barrier_wait(&wait_for_sender); //wait send thread to be ready

        //Setup our initial packet that will be sent (freed in graceful_shutdown)
        maloc_control_packet(start_packet);
        memcpy(start_packet, &control_packet_defaults ,sizeof(*start_packet));
        send_new_packet(&c, start_packet, PACKET_ONLY_SEND);

        pthread_create(&packet_receive_thread, NULL, receive_display_packets, &c);

//        if the radio is not already on try to turn it on
        if (check_radio_rx(&c) == false) {

                if (c.send.dtr_pin_for_on == true) {
                        int give_up = 0;
                        while (set_power_button(&c) != 0 && c.send.keep_alive) {
                                give_up++;
                                log_msg(RT8900_ERROR, "FAILED TO TURN ON THE RADIO AFTER %d trys\n", give_up);
                                if (give_up >= TURN_ON_RADIO_TRYS) {
                                        break;
                                }
                                sleep(1);
                        }
                } else {
                        log_msg(RT8900_INFO, "Waiting for radio to be turned on (no time out)\n");
                        while (check_radio_rx(&c) == false && c.send.keep_alive) {};
                }
        }

        if (check_radio_rx(&c) == true) {
                log_msg(RT8900_INFO, "Waiting for radio to boot...\n");
                sleep(3);
                user_prompt(&c, start_packet); //returns on user exit
        }


        graceful_shutdown(0);

        pthread_barrier_destroy(&wait_for_sender);
        pthread_join(packet_sender_thread, NULL);
        pthread_join(packet_receive_thread, NULL);
        return 0;
}
