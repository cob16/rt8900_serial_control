//
// Created by cormac on 24/02/17.
//

#ifndef RT8900_SERIAL_CONTROL_SERIAL_H
#define RT8900_SERIAL_CONTROL_SERIAL_H

#include <stdbool.h>
#include "control_packet.h"

typedef struct {
    CONTROL_PACKET **packet;
    bool keep_alive;
    char* serial_path;
    int serial_fd;
}SERIAL_CFG;

#endif //RT8900_SERIAL_CONTROLL_SERIAL_H
