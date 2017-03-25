//
// Created by cormac on 17/02/17.
//

#ifndef RT8900_SERIAL_CONTROL_PACKET_H
#define RT8900_SERIAL_CONTROL_PACKET_H

#include <sys/queue.h>
#include <stdbool.h>

#include "packet.h"

//the observed standard packet gap with
#define MILLISECONDS_BETWEEN_PACKETS_STANDARD 3
//the time that the radio waits to account for denounce effect on physical buttons + 1 second for safty
#define MILLISECONDS_DEBOUNCE_WAIT 51

#define DEFAULT_VOLUME 0x1f //25% volume

#define maloc_control_packet(pointer_name) struct control_packet *(pointer_name) = (struct control_packet*) malloc(sizeof(*(pointer_name)));

enum main_menu_buttons {
    NOT_PRESSED      = 0,
    R_ENCODER_BUTTON = 1,
    L_ENCODER_BUTTON = (1 << 1),
    SET_BUTTON       = (1 << 2),
    WIRES_BUTTON     = (1 << 3),
};

enum radios {
    RADIO_LEFT,
    RADIO_RIGHT,
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
    PACKET_BYTE right_buttons; // (via voltage divider)
    PACKET_BYTE left_buttons;  // (via voltage divider)
    PACKET_BYTE main_buttons;        // L/R encoder, set, and wires buttons
    PACKET_BYTE hyper_mem_buttons;   //hyper memory buttons
};

//values in order to select a row
enum voltage_divider_row_values {
    VOLTAGE_DEVIDER_INDEX_0 = 0X00, //0
    VOLTAGE_DEVIDER_INDEX_1 = 0X1A, //26
    VOLTAGE_DEVIDER_INDEX_2 = 0X32, //50
    VOLTAGE_DEVIDER_INDEX_3 = 0X4C, //76
    VOLTAGE_DEVIDER_INDEX_4 = 0X64, //100
    VOLTAGE_DEVIDER_NONE = 0X7F,    //127
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


#define BUTTON_NONE_VALUE {VOLTAGE_DEVIDER_NONE, VOLTAGE_DEVIDER_NONE}

#define BUTTON_1_VALUE {VOLTAGE_DEVIDER_INDEX_0, VOLTAGE_DEVIDER_INDEX_1}
#define BUTTON_2_VALUE {VOLTAGE_DEVIDER_INDEX_0, VOLTAGE_DEVIDER_INDEX_2}
#define BUTTON_3_VALUE {VOLTAGE_DEVIDER_INDEX_0, VOLTAGE_DEVIDER_INDEX_3}
#define BUTTON_A_VALUE {VOLTAGE_DEVIDER_INDEX_0, VOLTAGE_DEVIDER_INDEX_4}

#define BUTTON_UP_VALUE {VOLTAGE_DEVIDER_INDEX_1, VOLTAGE_DEVIDER_INDEX_0}
#define BUTTON_4_VALUE {VOLTAGE_DEVIDER_INDEX_1, VOLTAGE_DEVIDER_INDEX_1}
#define BUTTON_5_VALUE {VOLTAGE_DEVIDER_INDEX_1, VOLTAGE_DEVIDER_INDEX_2}
#define BUTTON_6_VALUE {VOLTAGE_DEVIDER_INDEX_1, VOLTAGE_DEVIDER_INDEX_3}
#define BUTTON_B_VALUE {VOLTAGE_DEVIDER_INDEX_1, VOLTAGE_DEVIDER_INDEX_4}

#define BUTTON_DOWN_VALUE {VOLTAGE_DEVIDER_INDEX_2, VOLTAGE_DEVIDER_INDEX_0}
#define BUTTON_7_VALUE {VOLTAGE_DEVIDER_INDEX_2, VOLTAGE_DEVIDER_INDEX_1}
#define BUTTON_8_VALUE {VOLTAGE_DEVIDER_INDEX_2, VOLTAGE_DEVIDER_INDEX_2}
#define BUTTON_9_VALUE {VOLTAGE_DEVIDER_INDEX_2, VOLTAGE_DEVIDER_INDEX_3}
#define BUTTON_C_VALUE {VOLTAGE_DEVIDER_INDEX_2, VOLTAGE_DEVIDER_INDEX_4}

#define BUTTON_X_VALUE {VOLTAGE_DEVIDER_INDEX_3, VOLTAGE_DEVIDER_INDEX_1}
#define BUTTON_0_VALUE {VOLTAGE_DEVIDER_INDEX_3, VOLTAGE_DEVIDER_INDEX_2}
#define BUTTON_HASH_VALUE {VOLTAGE_DEVIDER_INDEX_3, VOLTAGE_DEVIDER_INDEX_3}
#define BUTTON_D_VALUE {VOLTAGE_DEVIDER_INDEX_3, VOLTAGE_DEVIDER_INDEX_4}

#define BUTTON_P1_VALUE {VOLTAGE_DEVIDER_INDEX_4, VOLTAGE_DEVIDER_INDEX_1}
#define BUTTON_P2_VALUE {VOLTAGE_DEVIDER_INDEX_4, VOLTAGE_DEVIDER_INDEX_2}
#define BUTTON_P3_VALUE {VOLTAGE_DEVIDER_INDEX_4, VOLTAGE_DEVIDER_INDEX_3}
#define BUTTON_P4_VALUE {VOLTAGE_DEVIDER_INDEX_4, VOLTAGE_DEVIDER_INDEX_4}

const struct button_transmit_value KEYPAD_BUTTON_NONE;
const struct button_transmit_value KEYPAD_NUMBER_BUTTONS[10];

/// recommended defaults for the control packet
const struct control_packet control_packet_defaults;

/// used to get the struct as an array
typedef union {
    struct control_packet as_struct;
    PACKET_BYTE as_array[13];
} CONTROL_PACKET_INDEXED;

void set_keypad_button(struct control_packet *packet, const struct button_transmit_value *button);
void set_main_button(struct control_packet *packet, const enum main_menu_buttons button);
const struct button_transmit_value * button_from_int(int i);
signed char safe_int_char(int number);
int set_volume_left(struct control_packet *packet, int number);
int set_volume_right(struct control_packet *packet, int number);
int set_volume(struct control_packet *packet, int left, int right);
int set_squelch_left(struct control_packet *packet, int number);
int set_squelch_right(struct control_packet *packet, int number);
int set_squelch(struct control_packet *packet, int left, int right);
void ptt(struct control_packet *base_packet, int ptt);
void* send_control_packets(void *c);
void packet_debug(const struct control_packet *packet, CONTROL_PACKET_INDEXED *input_packet_arr);


#endif //RT8900_SERIAL_CONTROL_PACKET_H