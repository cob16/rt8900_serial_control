//
// Created by cormac on 24/02/17.
//

#ifndef RT8900_SERIAL_CONTROL_SERIAL_H
#define RT8900_SERIAL_CONTROL_SERIAL_H

#include <stdbool.h>
#include <sys/queue.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>


struct control_packet_q_node{
    struct control_packet *packet;
    enum pop_queue_behaviour free_packet; //should we free the packet once it is sent (default flase)
    TAILQ_ENTRY(control_packet_q_node) nodes; //link to next packet (for que)
};

enum pop_queue_behaviour {
    PACKET_SEND_THEN_FREE = 0,
    PACKET_SEND_ONLY = 1,
};

///create our packet queue struct
typedef TAILQ_HEAD(CONTROL_PACKET_Q_HEAD, control_packet_q_node) CONTROL_PACKET_Q_HEAD;

typedef struct {
    bool lazy;
    char *serial_path;
    pthread_barrier_t* initialised;
    struct CONTROL_PACKET_Q_HEAD *queue;
    int serial_fd;
    bool keep_alive;
} SERIAL_CFG;

void open_serial(SERIAL_CFG *cfg);

#endif //RT8900_SERIAL_CONTROLL_SERIAL_H
