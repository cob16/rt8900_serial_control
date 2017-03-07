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

///Creates the required packet to dial a number. they should then be added to the head of the queue*
int dial_number(struct control_packet *base_packet, int number)
{
        const struct button_transmit_value *button = button_from_int(number);
        set_button(base_packet, button);

        return 0;
}

///Creates the required packet to dial a number. they should then be added to the head of the queue*
int set_frequency(SERIAL_CFG *cfg, struct control_packet *base_packet, int number)
{
        //get the number of digits
        int num_digets = snprintf(NULL, 0, "%d", number);
        if (num_digets > 6){
                //this number
                printf("WARNING!: dialing a %d digit number! (%d) (Only 6 required for frequency inputs)", num_digets, number);
                return 1;
        }
        printf("dialing %d", number);

        //create a char array of the digits
        char digits[num_digets];
        snprintf(digits, num_digets + 1, "%d", number);

        //add packets that 'press' the seletced buttons
        int i;
        for (i=0; i<num_digets; i++){
                create_packet(dialnum)
                memcpy(dialnum, base_packet, sizeof(*base_packet));
                set_button(dialnum, button_from_int(digits[i] - '0'));
                send_new_packet(cfg, dialnum, 0);
                send_new_packet(cfg, base_packet, 1);
        }
        return 0;
}

///Starts sending controll packets as defined by SERIAL_CFG
void* send_control_packets(void *c)
{
        printf("-- STARTING CONTROL PACKET THREAD\n");
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
                                if (conf->verbose) {
                                        printf("-\n");
                                        packet_debug(current_packet, &packet_arr);
                                }
                                last_packet = current_packet;
                                packets_sent = 0;
                        } else {
                                printf("Sent %d packets\r",packets_sent);
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
                                printf("removed after %d packets sent\n", packets_sent);
                                TAILQ_REMOVE(conf->queue, current_node, nodes);
                                if (current_node->do_not_free != 1) {
                                        free(current_packet);
                                }
                                current_packet = NULL;
                                free(current_node);
                                current_node = NULL;
                        }
                } else {
                        printf("empty!\n");
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
void send_new_packet(SERIAL_CFG *config, struct control_packet *new_packet, int do_not_free)
{
        if (new_packet == NULL) {
                printf("NULL packet attempted to be added to QUEUE\n");
        } else {
                struct control_packet_q_node *node = (struct control_packet_q_node*) malloc(sizeof(*(node)));
                node->packet = new_packet;
                if (do_not_free) {
                        node->do_not_free = do_not_free;
                }
                TAILQ_INSERT_TAIL(config->queue, node, nodes);
                printf("ADDDED TO QUEUE \n");
        }
}


void packet_debug(const struct control_packet *packet, CONTROL_PACKET_INDEXED *packet_arr)
{
        int i;
        printf("\n");
        printf("--------------------------\n");
        printf("SERIAL CONTROL THREAD\nNew pointer address: %p \nNow sending:\n", packet);
        for (i = 0; i < sizeof((*packet_arr).as_array); i++) {
                print_char((*packet_arr).as_array[i].raw);
        }
        printf("--------------------------\n");
}