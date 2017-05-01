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

/// Options on if the packet added to the queue will be freed once sent
enum pop_queue_behaviour {
    PACKET_FREE_AFTER_SEND = 0,
    PACKET_ONLY_SEND = 1,
};

/// A internal, non intgrated queue struct for the packet queue
struct control_packet_q_node{
    struct control_packet *packet;
    enum pop_queue_behaviour free_packet; //should we free the packet once it is sent (default flase)
    TAILQ_ENTRY(control_packet_q_node) nodes; //link to next packet (for que)
};

/// Create our packet queue struct
typedef TAILQ_HEAD(CONTROL_PACKET_Q_HEAD, control_packet_q_node) CONTROL_PACKET_Q_HEAD;

void open_serial(int *fd, char **serial_path, bool *dtr_pin_for_on);

#endif //RT8900_SERIAL_CONTROLL_SERIAL_H
