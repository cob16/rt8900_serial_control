//
// Created by cormac on 24/02/17.
//

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>

#include <pthread.h>

#include "serial.h"
#include "packet.c"

void packet_debug(const struct control_packet *packet, CONTROL_PACKET_INDEXED *packet_arr);

/// Set our serial port attributes. Radio expects these constants
void set_serial_attributes(int fd)
{
        struct termios tty;
        memset(&tty, 0, sizeof(tty));

        //read in existing settings
        if (tcgetattr(fd, &tty) != 0) {
                printf("Failed reading terminal settings");
                exit(EXIT_FAILURE);
        }

        //set send and receive baud rates
        cfsetospeed (&tty, B19200);
        cfsetispeed (&tty, B19200);

        //sets out 8n1 without any other fancy options
        //Settings inspired from http://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c
        tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
        tty.c_cflag &= ~CSIZE;
        tty.c_cflag |= CS8;         /* 8-bit characters */
        tty.c_cflag &= ~PARENB;     /* no parity bit */
        tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
        tty.c_cflag &= ~CRTSCTS;    /* no hardware flow control */

//        TODO VERIFY that these settings will not be needed for reading
//        /* setup for non-canonical mode */
//        tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
//        tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
//        tty.c_oflag &= ~OPOST;
//
//        /* fetch bytes as they become available */
//        tty.c_cc[VMIN] = 1;
//        tty.c_cc[VTIME] = 1;

        //write our new settings
        if (tcsetattr(fd, TCSANOW, &tty) != 0) {
                printf("Failed to set terminal settings");
                exit(EXIT_FAILURE);
        }

}

/// Open and configure a serial port for sending and receiving from radio
void open_serial(SERIAL_CFG *cfg)
{
        int fd = 0;
        fd = open(cfg->serial_path, O_RDWR | O_NOCTTY | O_NDELAY );
        if (fd < 0) {
                printf("Error while opening serial_path %s\n", cfg->serial_path); // Just if you want user interface error control
                exit(EXIT_FAILURE);
        }

        set_serial_attributes(fd);

        cfg->serial_fd = fd;
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
                struct control_packet_q_node *node = malloc(sizeof(*(node)));
                node->packet = new_packet;
                if (do_not_free) {
                        node->do_not_free = do_not_free;
                }
                TAILQ_INSERT_TAIL(config->queue, node, nodes);
                printf("ADDDED TO QUEUE \n");
        }
}