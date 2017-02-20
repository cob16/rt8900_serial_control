//
// Created by cormac on 17/02/17.
//

#ifndef RT8900_SERIAL_CONTROLL_FOO_H
#define RT8900_SERIAL_CONTROLL_FOO_H

#endif //RT8900_SERIAL_CONTROLL_FOO_H

#include "packet.h"

#define MILLISECONDS_BETWEEN_PACKETS 3

int *example_function(int *n);

void set_to_3(int *n);

//To make a new CONTROL_PACKET please use "CONTROL_PACKET mypacket = control_packet_defaults"
//struct is in the order the packet requires
typedef struct {
    PACKET_BYTE encoder_right;        // <--twos complement, positive is num of clockwise turns
    PACKET_BYTE encoder_left;
    PACKET_BYTE ptt;                  // Push To Talk button, DATA_MAX_NUM for high (unpressed)
    PACKET_BYTE squelch_right;        // analog range
    PACKET_BYTE volume_control_right;
    PACKET_BYTE keypad_input_row;     //this x the keypad_column_input gets a number that is pressed
    PACKET_BYTE squelch_left;
    PACKET_BYTE volume_control_left;
    PACKET_BYTE keypad_input_column;
    PACKET_BYTE panel_buttons_right; // (via voltage divider)
    PACKET_BYTE panel_buttons_left;  // (via voltage divider)

} CONTROL_PACKET;

#define DEFAULT_VOLUME 0x3f

//const PACKET_BYTE encoder_right = {.section={.data=DATA_MIN_NUM, .check_num=SBO}};


//there are manny defaults that are 0 that we leave to be initialised as 0
const CONTROL_PACKET control_packet_defaults = {
        .encoder_right        = {.section = {.data = DATA_MIN_NUM, .check_num=SBO}},
        .ptt                  = {.section = {.data= DATA_MAX_NUM    }}, // off
        .volume_control_right = {.section = {.data = DEFAULT_VOLUME }}, // 50% volume
        .keypad_input_row     = {.section = {.data = DATA_MAX_NUM   }}, // no buttons being pressed TODO: verify if 0 or 127 is standard behavior
        .volume_control_left  = {.section = {.data = DEFAULT_VOLUME }}, // 50% volume
        .keypad_input_column  = {.section = {.data = DATA_MAX_NUM   }}, // no buttons being pressed TODO: verify if 0 or 127 is standard behavior
        .panel_buttons_right  = {.section = {.data = DATA_MAX_NUM   }}, // no buttons being pressed
        .panel_buttons_left   = {.section = {.data = DATA_MAX_NUM   }}, // no buttons being pressed
};

#pragma pack(1) //as we don't want space between our bits
typedef union {
    CONTROL_PACKET as_struct;
    PACKET_BYTE as_array[13];
} CONTROL_PACKET_INDEXED;
