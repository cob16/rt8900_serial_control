//
// Created by cormac on 14/03/17.
//

#ifndef RT8900_SERIAL_CONTROL_DISPLAY_PACKET_H
#define RT8900_SERIAL_CONTROL_DISPLAY_PACKET_H

//number of bytes in a display packet
#define DISPLAY_PACKET_SIZE 42

#define MS_PACKET_WAIT_TIME 25 //time for a full packet cycle

#include "packet.h"

struct display_packet {
        PACKET_BYTE arr[42];
};

enum power_level {
        low = 1,
        medium = 2,
        medium2 = 3,
        high = 4
};

struct radio_state_sides {
        int busy;
        char frequency;
        enum power_level power_level;
};


struct radio_state {
        struct radio_state_sides *main; //the currently selected pointer
        struct radio_state_sides left;
        struct radio_state_sides right;
};

#endif //RT8900_SERIAL_CONTROL_DISPLAY_PACKET_H