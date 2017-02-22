//
// Created by cormac on 17/02/17.
//

#ifndef RT8900_SERIAL_CONTROLL_FOO_H
#define RT8900_SERIAL_CONTROLL_FOO_H

#include <stdbool.h>
#include "packet.h"

#define MILLISECONDS_BETWEEN_PACKETS 3

#define DEFAULT_VOLUME 0x1f //25% volume

enum menu_buttons_values { //TODO This assumes that a 1 bit is the pressed postion WARNING: check this assumption
    NOT_PRESSED      = 0x00,
    WIRES_BUTTON     = 0x08,
    SET_BUTTON       = 0x10,
    L_ENCODER_BUTTON = 0x20,
    R_ENCODER_BUTTON = 0x40,
};

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
    PACKET_BYTE menu_buttons; // L/R encoder, set, and wires buttons
    PACKET_BYTE hyper_mem_buttons; //hyper memory buttons
} CONTROL_PACKET;

/// recommended defaults for the controll packet
const CONTROL_PACKET control_packet_defaults = {
        /*There are manny defaults that are 0 so we leave them as "{}"
        The elements are not addressed by name for c++ compatibility so we can test*/

        {.section = {.data = DATA_MIN_NUM, .check_num=SBO}}, //encoder_right | 0 turns
        {},                                     //encoder_left          | 0 turns
        {.section = {.data= DATA_MAX_NUM    }}, // ptt                  | set to high (off)
        {},                                     //squelch_right         | 0%
        {.section = {.data = DEFAULT_VOLUME }}, // volume_control_right | set to 25% volume
        {.section = {.data = DATA_MAX_NUM   }}, // keypad_input_row     | no buttons being pressed TODO: verify if 0 or 127 is standard behavior
        {},                                     // squelch_left         | 0%
        {.section = {.data = DEFAULT_VOLUME }}, // volume_control_left  | set to 25% volume
        {.section = {.data = DATA_MAX_NUM   }}, // keypad_input_column  | full is no buttons being pressed TODO: verify if 0 or 127 is standard behavior
        {.section = {.data = DATA_MAX_NUM   }}, // panel_buttons_right  | full is no buttons being pressed
        {.section = {.data = DATA_MAX_NUM   }}, // panel_buttons_left   | full is no buttons being pressed
        {.section = {.data = NOT_PRESSED    }}, // menu_buttons         |
        {},                                     // hyper_mem_buttons    |
};

#pragma pack(1) //as we don't want space between our bits
///used to get the struct as an array
typedef union {
    CONTROL_PACKET as_struct;
    PACKET_BYTE as_array[13];
} CONTROL_PACKET_INDEXED;

CONTROL_PACKET * make_packet();

typedef struct {
    CONTROL_PACKET** packet_pp; // we use a pointer pointer so we can update the pointer latter
    bool keep_alive;
} CONFIG;

CONFIG make_config(CONTROL_PACKET *packet);
void send_new_packet(CONFIG *config, CONTROL_PACKET *new_packet);

#endif //RT8900_SERIAL_CONTROLL_FOO_H