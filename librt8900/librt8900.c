//
// Created by cormac on 25/03/17.
//

#include "librt8900.h"
#include "serial.h"

#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>

///adds a control_packet (pointer) to the send queue, should only be called once the queu h
void send_new_packet(SERIAL_CFG *config, struct control_packet *new_packet, enum pop_queue_behaviour free_choice)
{
        if (new_packet == NULL) {
                log_msg(RT8900_ERROR, "NULL packet attempted to be added to QUEUE\n");
        } else {
                struct control_packet_q_node *node = (struct control_packet_q_node*) malloc(sizeof(*(node)));
                node->packet = new_packet;
                node->free_packet = free_choice;

                TAILQ_INSERT_TAIL(config->send.queue, node, nodes);
                log_msg(RT8900_TRACE, "ADDDED TO QUEUE \n");
        }
}

///Starts sending control packets as defined by SERIAL_CFG
void* send_control_packets(void *c)
{
        log_msg(RT8900_TRACE, "-- STARTING CONTROL PACKET THREAD\n");
        SERIAL_CFG *conf = (SERIAL_CFG*) c;
        open_serial(&(conf->serial_fd), &(conf->serial_path), &(conf->send.dtr_pin_for_on)); //setup our serial connection

        //setup queue
        CONTROL_PACKET_Q_HEAD head;
        TAILQ_INIT(&head);
        conf->send.queue = &head;
        conf->send.keep_alive = true;

        useconds_t us_between_packets;
        int repeat_packets;

        //sets where to emulate the head of the radio or just send a little as possible (without loss of speed)
        if (conf->send.lazy_sending) {
                us_between_packets = MILLISECONDS_DEBOUNCE_WAIT * 1000;
                repeat_packets = 1;
        } else {
                us_between_packets = MILLISECONDS_BETWEEN_PACKETS_STANDARD * 1000;
                repeat_packets = 8;
        }

        struct control_packet_q_node *current_node = NULL;
        struct control_packet *current_packet = NULL;
        struct control_packet *last_packet = NULL;
        int packets_sent = 0;

        pthread_barrier_wait(conf->send.initialised);
        while (conf->send.keep_alive) {
                if  (!TAILQ_EMPTY(&head)) {
                        current_node = TAILQ_FIRST(conf->send.queue);
                        current_packet = current_node->packet;
                        CONTROL_PACKET_INDEXED packet_arr = {.as_struct = *current_packet};

                        //print debug
                        if (current_packet != last_packet) {
                                log_msg(RT8900_TRACE, "-\n");
                                if (rt8900_verbose_level >= RT8900_TRACE) {
                                        packet_debug(current_packet, &packet_arr);
                                }
                                last_packet = current_packet;
                                packets_sent = 0;
                        }

                        //SEND THE PACKET
                        do {
                                write(conf->serial_fd, packet_arr.as_array, sizeof(packet_arr.as_array));
                                tcdrain(conf->serial_fd); //wait for the packet to send
                                packets_sent++;
#ifdef _WIN32
                                Sleep(us_between_packets);
#else
                                usleep(us_between_packets);
#endif
                        } while (packets_sent < repeat_packets);

                        //move to the next packet and clean up
                        if (current_node->nodes.tqe_next != NULL) { //pop if there is more to send
                                log_msg(RT8900_TRACE, "removed after %d packets sent\n", packets_sent);
                                TAILQ_REMOVE(conf->send.queue, current_node, nodes);
                                if (current_node->free_packet == PACKET_FREE_AFTER_SEND) {
                                        free(current_packet);
                                }
                                packets_sent = 0;
                                current_packet = NULL;
                                free(current_node);
                                current_node = NULL;
                        }
                } else {
                        log_msg(RT8900_TRACE, "Send queue is empty!\n");
                }


        }

        //clean out our queue incase of shutdown
        while (!TAILQ_EMPTY(conf->send.queue)) {
                current_node = TAILQ_FIRST(conf->send.queue);
                current_packet = current_node->packet;
                TAILQ_REMOVE(conf->send.queue, current_node, nodes);
                free(current_packet);
                current_packet = NULL;
        }

        return NULL;
}

void* receive_display_packets(void *c)
{
        SERIAL_CFG *config = (SERIAL_CFG*) c;
        config->receive.keep_alive = true;

        // allows our mutex to be locked across threads
        pthread_mutexattr_t shared;
        pthread_mutexattr_init(&shared);
        pthread_mutexattr_setpshared(&shared, PTHREAD_PROCESS_SHARED);

        if (pthread_mutex_init(&(config->receive.raw_packet_lock), &shared) != 0)
        {
                log_msg(RT8900_ERROR, "mutex init failed\n");
                exit(EXIT_FAILURE);
        }

        //open serial if not already open
        if (config->serial_fd  < 1) {
                //setup our serial connection if not already done
                open_serial(&(config->serial_fd), &(config->serial_path), &(config->send.dtr_pin_for_on));
        }

        tcflush(config->serial_fd, TCIFLUSH); //flush any existing buffer

        while (config->receive.keep_alive) {
                usleep(MS_PACKET_WAIT_TIME * 1000); //enough time for at least 1 packet to be sent

                // Initialize all the file descriptor sets
                fd_set read_fds, write_fds, except_fds;
                FD_ZERO(&read_fds);
                FD_ZERO(&write_fds);
                FD_ZERO(&except_fds);
                FD_SET(config->serial_fd, &read_fds);

                struct timeval timeout;
                timeout.tv_sec = 1;
                timeout.tv_usec = 0;

                // Wait for input to become ready or until the time out; the first parameter is
                // 1 more than the largest file descriptor in any of the sets
                if (select(config->serial_fd + 1, &read_fds, &write_fds, &except_fds, &timeout) == 1)
                {
                        pthread_mutex_lock(&(config->receive.raw_packet_lock));
                        read(config->serial_fd, &(config->receive.latest_raw_packet), DISPLAY_PACKET_SIZE);
                        pthread_mutex_unlock(&(config->receive.raw_packet_lock));
                        config->receive.radio_seen = true;
                }
                else
                {
                        log_msg(RT8900_WARNING, "NO DATA RECEIVIED FROM RADIO!\n");
                }
        }

        pthread_mutex_destroy(&(config->receive.raw_packet_lock));

        return 0;
}

int get_display_packet(SERIAL_CFG *config, struct display_packet *packet)
{
        unsigned char temp_buffer[DISPLAY_PACKET_SIZE];
        pthread_mutex_lock(&(config->receive.raw_packet_lock));
        memcpy(&temp_buffer, (config->receive.latest_raw_packet) , sizeof(temp_buffer));
        pthread_mutex_unlock(&(config->receive.raw_packet_lock));


        int start_of_packet_index = find_packet_start(temp_buffer, sizeof(temp_buffer));
        if (start_of_packet_index == -1 ) {
                log_msg(RT8900_ERROR, "PACKET was corrupt / unrecognised!\n");
                return 1;
        }
        insert_shifted_packet(packet, temp_buffer, DISPLAY_PACKET_SIZE, start_of_packet_index);
        return 0;
}


///Check that we are reciving from teh radio.
int check_radio_rx(SERIAL_CFG *config)
{
        return config->receive.radio_seen;
}

/// stwitches context to the desired radio,
/// you must first check you are not already on this mode else you will enter frequency edit mode!
int set_main_radio(SERIAL_CFG *cfg, struct control_packet *base_packet, enum radios side) {
        maloc_control_packet(switch_main);
        memcpy(switch_main, base_packet, sizeof(*switch_main));

        switch (side){
        case RADIO_LEFT:
                set_main_button(switch_main, L_ENCODER_BUTTON);
                break;
        case RADIO_RIGHT:
                set_main_button(switch_main, R_ENCODER_BUTTON);
                break;
        default:
                free(switch_main);
                return -1;
        }

        send_new_packet(cfg, switch_main, PACKET_FREE_AFTER_SEND);
        send_new_packet(cfg, base_packet, PACKET_ONLY_SEND);
        return 0;

}

///Adds the required packets to dial a number. they are then be added to the queue
int set_frequency(SERIAL_CFG *cfg, struct control_packet *base_packet, int number)
{
        //get the number of digits
        int num_digets = snprintf(NULL, 0, "%d", number);
        if (num_digets > 6){
                //this number
                log_msg(RT8900_WARNING, "WARNING!: dialing a %d digit number! (%d) (Only 6 required for frequency inputs)", num_digets, number);
                return 1;
        }
        log_msg(RT8900_INFO, "dialing %d\n", number);

        //create a char array of the digits
        char digits[num_digets];
        snprintf(digits, num_digets + 1, "%d", number);

        //We assume smaller numbers are missing front 0's so we dial them here
        int i;
        for (i=0; i<(6 - num_digets); i++) {
                maloc_control_packet(dialnum);
                memcpy(dialnum, base_packet, sizeof(*base_packet));
                set_keypad_button(dialnum, button_from_int(0));
                send_new_packet(cfg, dialnum, PACKET_FREE_AFTER_SEND);
                send_new_packet(cfg, base_packet, PACKET_ONLY_SEND);
                log_msg(RT8900_DEBUG, "dialing 0\n");
        }

        //dial the seletced buttons
        for (i=0; i<num_digets; i++){
                maloc_control_packet(dialnum);
                memcpy(dialnum, base_packet, sizeof(*base_packet));
                set_keypad_button(dialnum, button_from_int(digits[i] - '0'));
                send_new_packet(cfg, dialnum, PACKET_FREE_AFTER_SEND);
                send_new_packet(cfg, base_packet, PACKET_ONLY_SEND);
                log_msg(RT8900_DEBUG, "dialing %d\n", digits[i] - '0');
        }
        return 0;
}


void wait_to_send(const SERIAL_CFG *cfg)
{
        if (cfg->send.keep_alive == true && !TAILQ_EMPTY(cfg->send.queue)) {
                while (cfg->send.queue->tqh_first->nodes.tqe_next != NULL) {
                        usleep(MILLISECONDS_DEBOUNCE_WAIT * 1000);
                };
                usleep(MILLISECONDS_DEBOUNCE_WAIT * 1000);
        }
}

///Adds the required packets to dial a number. they are then be added to the queue
int set_left_power_level(SERIAL_CFG *cfg, struct control_packet *base_packet, enum rt8900_power_level power_level)
{
        //todo when diget reading is completed this will need to be updated to allow selection of med1 and med1 levels

        if (!VALID_POWER_LEVEL(power_level)) {
                return 1;
        }

        struct display_packet packet;
        get_display_packet(cfg, &packet);
        struct radio_state state;
        read_power_fuzzy(&packet, &state);

        while(state.left.power_level != power_level && cfg->send.keep_alive == true) {
                maloc_control_packet(power_press);
                memcpy(power_press, base_packet, sizeof(*base_packet));
                set_left_button(power_press, LEFT_LOW);

                send_new_packet(cfg, power_press, PACKET_FREE_AFTER_SEND);
                send_new_packet(cfg, base_packet, PACKET_ONLY_SEND);
                wait_to_send(cfg);

                sleep(1); //the radio is very inconsitant on change time this is here for some safty
                get_display_packet(cfg, &packet);
                read_power_fuzzy(&packet, &state);
        }

        return 0;
}

///press the 'low' button untill the desired power level is set
int set_right_power_level(SERIAL_CFG *cfg, struct control_packet *base_packet, enum rt8900_power_level power_level)
{
        //todo when diget reading is completed this will need to be updated to allow selection of med1 and med1 levels

        if (!VALID_POWER_LEVEL(power_level)) {
                return 1;
        }

        struct display_packet packet;
        get_display_packet(cfg, &packet);
        struct radio_state state;
        read_power_fuzzy(&packet, &state);

        while(state.right.power_level != power_level && cfg->send.keep_alive == true) {
                maloc_control_packet(power_press);
                memcpy(power_press, base_packet, sizeof(*base_packet));
                set_right_button(power_press, RIGHT_LOW);

                send_new_packet(cfg, power_press, PACKET_FREE_AFTER_SEND);
                send_new_packet(cfg, base_packet, PACKET_ONLY_SEND);
                wait_to_send(cfg);

                /* We do not up now what the packet latency is between sending a cmd and getting a updated screen
                 * so we will wait a second to garentee the program can se the update */
                // todo find a way to work around this
                sleep(1);

                get_display_packet(cfg, &packet);
                read_power_fuzzy(&packet, &state);
        }

        return 0;
}

///sets the dtr pin low for one second to trigger radio on
int set_power_button(SERIAL_CFG *cfg)
{
        if (cfg->send.dtr_pin_for_on == false) {
                log_msg(RT8900_ERROR, "Power on flag not provided, please see help output to enable");
                return 1;
        }

        int RTS_flag = TIOCM_DTR;
        ioctl(cfg->serial_fd, TIOCMGET, &RTS_flag);
        RTS_flag &= ~TIOCM_DTR;
        ioctl(cfg->serial_fd, TIOCMSET, &RTS_flag);

        sleep(1);

        ioctl(cfg->serial_fd, TIOCMGET, &RTS_flag);
        RTS_flag |= TIOCM_DTR;
        ioctl(cfg->serial_fd, TIOCMSET, &RTS_flag);

        if (check_radio_rx(cfg) != 1) {
                log_msg(RT8900_ERROR, "Failed to turn on radio\n");
                return 1;
        }

        return 0;
}