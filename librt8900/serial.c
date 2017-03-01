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

#include "serial.h"
#include "packet.c"
#include "control_packet.h"

void packet_debug(const struct CONTROL_PACKET *packet, CONTROL_PACKET_INDEXED *packet_arr);

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

//void* send_control_packets(void *c)
//{
//        printf("-- STARTING CONTROL PACKET THREAD\n");
//        SERIAL_CFG *conf = (SERIAL_CFG*) c;
//
//        CONTROL_PACKET** packet_ptr = conf->packet;
//        CONTROL_PACKET *last_ptr = NULL;
//
//        open_serial(conf); //setup our serial connection
//
//        int packets_sent = 0; //TODO does it matter that this counter will overflow in 246 days?
//        int packets_sent_to_user = 0;
//
//        printf("-- Now Sending --\n");
//        while (conf->keep_alive) {
//                CONTROL_PACKET *packet = *packet_ptr; //we dereference each time so that we can change the pointer elsewhere
//                CONTROL_PACKET_INDEXED packet_arr = {.as_struct = *packet};
//                if (packet != last_ptr) {
//                        packet_debug(packet, &packet_arr);
//                        last_ptr = packet;
//                        packets_sent = 0;
//                }else{
//                        packets_sent++;
//                        if (packets_sent - packets_sent_to_user > 100){
//                                printf("Sent: %3d Packets\r", packets_sent); /* \r returns the caret to the line start */
//                                fflush(stdout);
//                                packets_sent_to_user = packets_sent;
//                        }
//                }
//
//                //SEND THE PACKET
//                write(conf->serial_fd, packet_arr.as_array, sizeof(packet_arr.as_array));
//                tcdrain(conf->serial_fd); //wait for the packet to send
//                #ifdef _WIN32
//                Sleep(MILLISECONDS_BETWEEN_PACKETS);
//                #else
//                usleep(MILLISECONDS_BETWEEN_PACKETS * 1000);  /* sleep 3 milliSeconds */
//                #endif
//        }
//        printf("\n");
//        printf("-- ENDING CONTROL PACKET THREAD\n");
//        return NULL;
//}

void packet_debug(const struct CONTROL_PACKET *packet, CONTROL_PACKET_INDEXED *packet_arr)
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
        CONTROL_PACKET_Q head;
        TAILQ_INIT(&head);
        conf->queue = &head;

        struct CONTROL_PACKET *current_packet = NULL;
        while (conf->keep_alive) {

                if  (!TAILQ_EMPTY(&head)) {
                        current_packet = TAILQ_FIRST(conf->queue);
                        CONTROL_PACKET_INDEXED packet_arr = {.as_struct = *current_packet};

                        packet_debug(current_packet, &packet_arr);

                        //SEND THE PACKET
                        write(conf->serial_fd, packet_arr.as_array, sizeof(packet_arr.as_array));
                        tcdrain(conf->serial_fd); //wait for the packet to send

                        if (current_packet->nodes.tqe_next != NULL) { //pop and free if there is more to send
                                TAILQ_REMOVE(conf->queue, current_packet, nodes);
                                free(current_packet);
                                current_packet = NULL;
                        }
                } else {
                        printf("READY FOR INPUT \n");
                }
                #ifdef _WIN32
                                Sleep(MILLISECONDS_BETWEEN_PACKETS);
                #else
                                usleep(MILLISECONDS_BETWEEN_PACKETS * 1000);  /* sleep 3 milliSeconds */
                #endif
        }

        //clean out our queue incase of shutdown
        while (!TAILQ_EMPTY(conf->queue)) {
                current_packet = TAILQ_FIRST(conf->queue);
                TAILQ_REMOVE(conf->queue, current_packet, nodes);
                free(current_packet);
                current_packet = NULL;
        }
}

///adds a CONTROL_PACKET (pointer) to the send queue
void send_new_packet(SERIAL_CFG *config, struct CONTROL_PACKET *new_packet)
{
        while(config->queue == NULL){
                printf("BLOCKING send_new_packet as q does not yet exist (packet)");
        }
        TAILQ_INSERT_TAIL(config->queue, new_packet, nodes);
}