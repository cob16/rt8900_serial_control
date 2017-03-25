//
// Created by cormac on 25/03/17.
//

#include "librt8900.h"

#include <unistd.h>
#include <pthread.h>

void log_msg(enum rt8900_logging_level level, char const *fmt, ...)
{
        va_list args;
        va_start(args, fmt);

        if (rt8900_verbose >= level) {
                vfprintf(stderr, fmt, args);
        }
        va_end(args);
}

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
        open_serial(&(conf->serial_fd), &(conf->serial_path)); //setup our serial connection

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
                                if (rt8900_verbose >= RT8900_TRACE) {
                                        packet_debug(current_packet, &packet_arr);
                                }
                                last_packet = current_packet;
                                packets_sent = 0;
                        } else {
                                log_msg(RT8900_TRACE, "Sent %d packets\r",packets_sent);
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
                                current_packet = NULL;
                                free(current_node);
                                current_node = NULL;
                        }
                } else {
                        log_msg(RT8900_TRACE, "empty!\n");
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

int get_display_packet(SERIAL_CFG *config, struct display_packet *packet)
{
        tcflush(config->serial_fd, TCIFLUSH); //flush the buffer to get the latest data
        usleep(MS_PACKET_WAIT_TIME * 1000); //enough time for at least 1 packet to be sent

        if (check_radio_rx(config->serial_fd) != 0) {
                return 1;
        }

        unsigned char buffer[DISPLAY_PACKET_SIZE];

        int recived_bytes = read(config->serial_fd, &buffer, DISPLAY_PACKET_SIZE);
        int start_of_packet_index = find_packet_start(buffer, sizeof(buffer));

        if (recived_bytes != DISPLAY_PACKET_SIZE || start_of_packet_index == -1 ) {
                log_msg(RT8900_ERROR, "PACKET was corrupt / unrecognised!\n");
                return 1;
        }
        insert_shifted_packet(packet, buffer, DISPLAY_PACKET_SIZE, start_of_packet_index);
        return 0;
}

int get_state(SERIAL_CFG *config, struct radio_state *state) {
        struct display_packet packet;
        get_display_packet(config, &packet);
        read_state_from_packet(&packet, state);
        return 1;
}

/// stwitches context to the desired radio,
/// you must first check you are not already on this mode else you will enter frequency edit mode!
int set_main_radio(SERIAL_CFG *cfg, struct control_packet *base_packet, enum radios side) {
        maloc_control_packet(switch_main)
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

        //add packets that 'press' the seletced buttons
        int i;
        for (i=0; i<num_digets; i++){
                maloc_control_packet(dialnum)
                memcpy(dialnum, base_packet, sizeof(*base_packet));
                set_keypad_button(dialnum, button_from_int(digits[i] - '0'));
                send_new_packet(cfg, dialnum, PACKET_FREE_AFTER_SEND);
                send_new_packet(cfg, base_packet, PACKET_ONLY_SEND);
        }
        return 0;
}