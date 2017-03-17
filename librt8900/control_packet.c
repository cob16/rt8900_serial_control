//
// Created by cormac on 01/03/17.
//
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <pthread.h>
#include <termios.h>
#include <sys/queue.h>
#include <unistd.h>

#include "control_packet.h"
#include "serial.c"
#include "packet.c"

void set_button(struct control_packet *packet, const struct button_transmit_value *button)
{
        packet->keypad_input_row.section.data    = button->row;
        packet->keypad_input_column.section.data = button->column;
}

///returns null if the number will not fit into the packet (7 bits)
signed char safe_int_char(int number)
{
        if(number < DATA_MIN_NUM) {
                return (signed char) DATA_MIN_NUM;

        } else if (number > DATA_MAX_NUM) {
                return (signed char) DATA_MAX_NUM;
        }
        return (signed char) number;
}

///set the volume between 0-127. 0 is mute.
int set_volume_left(struct control_packet *packet, int number)
{
        if (packet == NULL) {
                return 1;
        }
        packet->volume_control_left.section.data = safe_int_char(number);
        return 0;
}

///set the volume between 0-127. 0 is mute.
int set_volume_right(struct control_packet *packet, int number)
{
        if (packet == NULL) {
                return 1;
        }
        packet->volume_control_right.section.data = safe_int_char(number);
        return 0;
}

/// Set the left and right volume. between 0-127. 0 is mute.
int set_volume(struct control_packet *packet, int left, int right)
{
        int o = 0;
        o += set_volume_left(packet, left);
        o += set_volume_right(packet, right);
        return o;
}

/// Set the left squelch between 0-127.
///127 filters no noise.e.
int set_squelch_left(struct control_packet *packet, int number)
{
        if (packet == NULL) {
                return 1;
        }
        packet->squelch_left.section.data = safe_int_char(number);
        return 0;
}

/// Set the right squelch between 0-127.
///127 filters no noise.
int set_squelch_right(struct control_packet *packet, int number)
{
        if (packet == NULL) {
                return 1;
        }
        packet->squelch_right.section.data = safe_int_char(number);
        return 0;
}

/// Set the left and right squelch. between 0-127.
///127 filters no noise.
int set_squelch(struct control_packet *packet, int left, int right)
{
        int o = 0;
        o += set_squelch_left(packet, left);
        o += set_squelch_right(packet, right);
        return o;
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
                set_button(dialnum, button_from_int(digits[i] - '0'));
                send_new_packet(cfg, dialnum, PACKET_FREE_AFTER_SEND);
                send_new_packet(cfg, base_packet, PACKET_ONLY_SEND);
        }
        return 0;
}

///Starts sending control packets as defined by SERIAL_CFG
void* send_control_packets(void *c)
{
        log_msg(RT8900_TRACE, "-- STARTING CONTROL PACKET THREAD\n");
        SERIAL_CFG *conf = (SERIAL_CFG*) c;
        open_serial(conf); //setup our serial connection

        //setup queue
        CONTROL_PACKET_Q_HEAD head;
        TAILQ_INIT(&head);
        conf->queue = &head;
        conf->keep_alive = true;

        useconds_t us_between_packets;
        int repeat_packets;

        //sets where to emulate the head of the radio or just send a little as possible (without loss of speed)
        if (conf->lazy) {
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

        pthread_barrier_wait(conf->initialised);
        while (conf->keep_alive) {
                if  (!TAILQ_EMPTY(&head)) {
                        current_node = TAILQ_FIRST(conf->queue);
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
                                TAILQ_REMOVE(conf->queue, current_node, nodes);
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
        while (!TAILQ_EMPTY(conf->queue)) {
                current_node = TAILQ_FIRST(conf->queue);
                current_packet = current_node->packet;
                TAILQ_REMOVE(conf->queue, current_node, nodes);
                free(current_packet);
                current_packet = NULL;
        }

        close(conf->serial_fd);

        return NULL;
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

                TAILQ_INSERT_TAIL(config->queue, node, nodes);
                log_msg(RT8900_TRACE, "ADDDED TO QUEUE \n");
        }
}


void packet_debug(const struct control_packet *packet, CONTROL_PACKET_INDEXED *input_packet_arr)
{
        //Create a array to examine the packet if one was not supplied
        CONTROL_PACKET_INDEXED *packet_array = input_packet_arr;
        if (packet_array == NULL) {
                CONTROL_PACKET_INDEXED new_array = {.as_struct = *packet};
                packet_array = &new_array;
        }

        int i;
        printf("\n");
        printf("--------------------------\n");
        printf("SERIAL CONTROL THREAD\nNew pointer address: %p \nNow sending:\n", packet);
        for (i = 0; i < sizeof((*packet_array).as_array); i++) {
                print_char((*packet_array).as_array[i].raw);
        }
        printf("--------------------------\n");
}
