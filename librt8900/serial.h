//
// Created by cormac on 24/02/17.
//

#ifndef RT8900_SERIAL_CONTROL_SERIAL_H
#define RT8900_SERIAL_CONTROL_SERIAL_H

#include <stdbool.h>
#include <sys/queue.h>

#include "control_packet.h"

typedef struct {
    bool verbose;
    char *serial_path;
    pthread_barrier_t* initialised;
    CONTROL_PACKET_Q *queue;
    int serial_fd;
    bool keep_alive;
} SERIAL_CFG;

#endif //RT8900_SERIAL_CONTROLL_SERIAL_H
