//
// Created by cormac on 14/03/17.
//

#ifndef RT8900_SERIAL_CONTROL_DISPLAY_PACKET_H
#define RT8900_SERIAL_CONTROL_DISPLAY_PACKET_H

//number of bytes in a display packet
#define DISPLAY_PACKET_SIZE 42


#include "packet.h"

struct display_packet {
    PACKET_BYTE arr[42];
};

#endif //RT8900_SERIAL_CONTROL_DISPLAY_PACKET_H