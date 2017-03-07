//
// Created by cormac on 17/02/17.
//

#ifndef RT8900_SERIAL_CONTROL_FOO_H
#define RT8900_SERIAL_CONTROL_FOO_H

#include <sys/queue.h>
#include <stdbool.h>
#include "packet.h"

//the observed standard packet gap with
#define MILLISECONDS_BETWEEN_PACKETS_STANDARD 3
//the time that the radio waits to account for denounce effect on physical buttons + 1 second for safty
#define MILLISECONDS_DEBOUNCE_WAIT 51


#define DEFAULT_VOLUME 0x1f //25% volume

enum misc_menu_buttons { //TODO This assumes that a 1 bit is the pressed position WARNING: check this assumption
    NOT_PRESSED      = 0x00,
    WIRES_BUTTON     = 0x08,
    SET_BUTTON       = 0x10,
    L_ENCODER_BUTTON = 0x20,
    R_ENCODER_BUTTON = 0x40,
};

enum left_menu_buttons {
    LEFT_LOW = 0x00,
    LEFT_VM  = 0x20,
    LEFT_HM  = 0x40,
    LEFT_SCN = 0x60
};

enum right_menu_buttons { //the right buttons are mirrored in value from the left
    RIGHT_LOW = LEFT_SCN,
    RIGHT_VM  = LEFT_HM,
    RIGHT_HM  = LEFT_VM,
    RIGHT_SCN = LEFT_LOW
};

//To make a new control_packet please use "control_packet mypacket = control_packet_defaults"
//struct is in the order the packet requires
struct control_packet{
    PACKET_BYTE encoder_right;        // <--twos complement, positive is num of clockwise turns
    PACKET_BYTE encoder_left;
    PACKET_BYTE ptt;                  // Push To Talk button, DATA_MAX_NUM for high (unpressed)
    PACKET_BYTE squelch_right;        // analog range
    PACKET_BYTE volume_control_right;
    PACKET_BYTE keypad_input_row;     //this x the keypad_column_input gets a number that is pressed
    PACKET_BYTE volume_control_left;
    PACKET_BYTE squelch_left;
    PACKET_BYTE keypad_input_column;
    PACKET_BYTE panel_buttons_right; // (via voltage divider)
    PACKET_BYTE panel_buttons_left;  // (via voltage divider)
    PACKET_BYTE menu_buttons;        // L/R encoder, set, and wires buttons
    PACKET_BYTE hyper_mem_buttons;   //hyper memory buttons
};

//values in order to select a row
enum voltage_divider_row_values {
    VOLTAGE_DEVIDER_INDEX_0 = 0X00,//0
    VOLTAGE_DEVIDER_INDEX_1 = 0X1A,//26
    VOLTAGE_DEVIDER_INDEX_2 = 0X32,//50
    VOLTAGE_DEVIDER_INDEX_3 = 0X4C,//76
    VOLTAGE_DEVIDER_INDEX_4 = 0X64,//100
    VOLTAGE_DEVIDER_NONE = 0X7F,   //127
};

//used to store the required values in order to dial a handset button
struct button_transmit_value{
    signed char row;
    signed char column;
};

// table showing the layout of the button grid that is assigned to constants bellow
//
//| row/col | 0    | 1  | 2  | 3  | 4  |
//|---------|------|----|----|----|----|
//| 0       |      | 1  | 2  | 3  | A  |
//| 1       | UP   | 4  | 5  | 6  | B  |
//| 2       | DOWN | 7  | 8  | 9  | C  |
//| 3       |      | x  | 0  | #  | D  |
//| 4       |      | P1 | P2 | P3 | P4 |

//row first
#define DEFINE_BUTTON(name, row, columb) const struct button_transmit_value (name) = {(signed char) (row), (signed char) (columb)};

DEFINE_BUTTON(BUTTON_NONE, VOLTAGE_DEVIDER_NONE, VOLTAGE_DEVIDER_NONE)

DEFINE_BUTTON(BUTTON_1, VOLTAGE_DEVIDER_INDEX_0, VOLTAGE_DEVIDER_INDEX_1)
DEFINE_BUTTON(BUTTON_2, VOLTAGE_DEVIDER_INDEX_0, VOLTAGE_DEVIDER_INDEX_2)
DEFINE_BUTTON(BUTTON_3, VOLTAGE_DEVIDER_INDEX_0, VOLTAGE_DEVIDER_INDEX_3)
DEFINE_BUTTON(BUTTON_A, VOLTAGE_DEVIDER_INDEX_0, VOLTAGE_DEVIDER_INDEX_4)

DEFINE_BUTTON(BUTTON_UP, VOLTAGE_DEVIDER_INDEX_1, VOLTAGE_DEVIDER_INDEX_0)
DEFINE_BUTTON(BUTTON_4, VOLTAGE_DEVIDER_INDEX_1, VOLTAGE_DEVIDER_INDEX_1)
DEFINE_BUTTON(BUTTON_5, VOLTAGE_DEVIDER_INDEX_1, VOLTAGE_DEVIDER_INDEX_2)
DEFINE_BUTTON(BUTTON_6, VOLTAGE_DEVIDER_INDEX_1, VOLTAGE_DEVIDER_INDEX_3)
DEFINE_BUTTON(BUTTON_B, VOLTAGE_DEVIDER_INDEX_1, VOLTAGE_DEVIDER_INDEX_4)

DEFINE_BUTTON(BUTTON_DOWN, VOLTAGE_DEVIDER_INDEX_2, VOLTAGE_DEVIDER_INDEX_0)
DEFINE_BUTTON(BUTTON_7, VOLTAGE_DEVIDER_INDEX_2, VOLTAGE_DEVIDER_INDEX_1)
DEFINE_BUTTON(BUTTON_8, VOLTAGE_DEVIDER_INDEX_2, VOLTAGE_DEVIDER_INDEX_2)
DEFINE_BUTTON(BUTTON_9, VOLTAGE_DEVIDER_INDEX_2, VOLTAGE_DEVIDER_INDEX_3)
DEFINE_BUTTON(BUTTON_C, VOLTAGE_DEVIDER_INDEX_2, VOLTAGE_DEVIDER_INDEX_4)

DEFINE_BUTTON(BUTTON_X, VOLTAGE_DEVIDER_INDEX_3, VOLTAGE_DEVIDER_INDEX_1)
DEFINE_BUTTON(BUTTON_0, VOLTAGE_DEVIDER_INDEX_3, VOLTAGE_DEVIDER_INDEX_2)
DEFINE_BUTTON(BUTTON_HASH, VOLTAGE_DEVIDER_INDEX_3, VOLTAGE_DEVIDER_INDEX_3)
DEFINE_BUTTON(BUTTON_D, VOLTAGE_DEVIDER_INDEX_3, VOLTAGE_DEVIDER_INDEX_4)

DEFINE_BUTTON(BUTTON_P1, VOLTAGE_DEVIDER_INDEX_4, VOLTAGE_DEVIDER_INDEX_1)
DEFINE_BUTTON(BUTTON_P2, VOLTAGE_DEVIDER_INDEX_4, VOLTAGE_DEVIDER_INDEX_2)
DEFINE_BUTTON(BUTTON_P3, VOLTAGE_DEVIDER_INDEX_4, VOLTAGE_DEVIDER_INDEX_3)
DEFINE_BUTTON(BUTTON_P4, VOLTAGE_DEVIDER_INDEX_4, VOLTAGE_DEVIDER_INDEX_4)

const struct button_transmit_value *number_buttons[10] = { &BUTTON_0, &BUTTON_1, &BUTTON_2, &BUTTON_3, &BUTTON_4,
                                                           &BUTTON_5, &BUTTON_6, &BUTTON_7, &BUTTON_8, &BUTTON_9};

const struct button_transmit_value * button_from_int(int i)
{
        if (-1 < i && i < 10) {
                return number_buttons[i];
        } else {
                //"WARNING: Invalid range given to button_from_int was: %d (must be 0-10) ",i); todo: log this
                return &BUTTON_NONE;
        }
}

/// recommended defaults for the control packet
const struct control_packet control_packet_defaults = {
        /*There are manny defaults that are 0 so we leave them as "{}"
        The elements are not addressed by name for c++ compatibility so we can test*/

        {.section = {.data = DATA_MIN_NUM, .check_num=SBO}}, //encoder_right | 0 turns
        {},                                         //encoder_left          | 0 turns
        {.section = {.data= DATA_MAX_NUM}},         // ptt                  | set to high (off)
        {},                                         //squelch_right         | 0%
        {.section = {.data = DEFAULT_VOLUME}},      // volume_control_right | set to 25% volume
        {.section = {.data = VOLTAGE_DEVIDER_NONE}},// keypad_input_row     | no buttons being pressed TODO: verify if 0 or 127 is standard behavior
        {.section = {.data = DEFAULT_VOLUME}},      // volume_control_left  | set to 25% volume
        {},                                         // squelch_left         | 0%
        {.section = {.data = VOLTAGE_DEVIDER_NONE}},// keypad_input_column  | full is no buttons being pressed TODO: verify if 0 or 127 is standard behavior
        {.section = {.data = DATA_MAX_NUM}},        // panel_buttons_right  | full is no buttons being pressed
        {.section = {.data = DATA_MAX_NUM}},        // panel_buttons_left   | full is no buttons being pressed
        {.section = {.data = NOT_PRESSED}},         // menu_buttons         |
        {},                                         // hyper_mem_buttons    |
};

/// used to get the struct as an array
typedef union {
    struct control_packet as_struct;
    PACKET_BYTE as_array[13];
} CONTROL_PACKET_INDEXED;


#endif //RT8900_SERIAL_CONTROL_FOO_H