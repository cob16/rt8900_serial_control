//
// Created by cormac on 14/03/17.
//

#ifndef RT8900_SERIAL_CONTROL_DISPLAY_PACKET_H
#define RT8900_SERIAL_CONTROL_DISPLAY_PACKET_H

#include "packet.h"
#include "serial.h"
#include "log.h"

//number of bytes in a display packet
#define DISPLAY_PACKET_SIZE 42

#define MS_PACKET_WAIT_TIME 25 //time for a full packet cycle

#define BIT_LOCATED_AT(byte, bit) (((byte) * 8) + (bit))

//misc
#define AUTO_POWER_OFF BIT_LOCATED_AT(0, 1)
#define KEYPAD_LOCK BIT_LOCATED_AT(30, 4)
#define SET BIT_LOCATED_AT(30, 0)
#define BD8600 BIT_LOCATED_AT(4, 0)

//left side
#define LEFT_BUISY  BIT_LOCATED_AT(12,2)
#define LEFT_MAIN  BIT_LOCATED_AT(36, 2)
#define LEFT_TX BIT_LOCATED_AT(36, 3)

#define LEFT_MINUS BIT_LOCATED_AT(36, 5)
#define LEFT_PLUSS BIT_LOCATED_AT(36, 4)
#define LEFT_DCS BIT_LOCATED_AT(9, 3)
#define LEFT_DEC BIT_LOCATED_AT(37, 0)
#define LEFT_ENC BIT_LOCATED_AT(36, 6)
#define LEFT_SKIP BIT_LOCATED_AT(37, 2)
#define LEFT_PMS BIT_LOCATED_AT(37, 1)

#define LEFT_POWER_LOW    BIT_LOCATED_AT(1, 6)
#define LEFT_POWER_MEDIUM BIT_LOCATED_AT(0, 2)
//#define LEFT_POWER_HIGH todo where is this?

//right side
#define RIGHT_BUISY BIT_LOCATED_AT(28,2)
#define RIGHT_MAIN BIT_LOCATED_AT(32, 0)
#define RIGHT_TX BIT_LOCATED_AT(32, 1)

#define RIGHT_MINUS BIT_LOCATED_AT(32, 3)
#define RIGHT_PLUSS BIT_LOCATED_AT(32, 2)
#define RIGHT_DCS BIT_LOCATED_AT(22, 2)
#define RIGHT_DEC BIT_LOCATED_AT(32, 5)
#define RIGHT_ENC BIT_LOCATED_AT(32, 4)
#define RIGHT_SKIP BIT_LOCATED_AT(33, 0)
#define RIGHT_PMS BIT_LOCATED_AT(32, 6)

#define RIGHT_POWER_LOW    BIT_LOCATED_AT(17, 6)
#define RIGHT_POWER_MEDIUM BIT_LOCATED_AT(16, 3)
//#define RIGHT_POWER_HIGH  todo where is this?


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
        int frequency;
        enum power_level power_level;
};


struct radio_state {
        struct radio_state_sides *main; //the currently selected pointer
        struct radio_state_sides left;
        struct radio_state_sides right;
};

void insert_shifted_packet(struct display_packet *packet, unsigned char buffer[], size_t buffer_length, int start_of_packet_index);
void read_busy(struct display_packet *packet, struct radio_state *state);
void read_main(struct display_packet *packet, struct radio_state *state);

int decode_14_segment(int segment_bitmask);
int display_packet_read(struct display_packet *packet, int bit_number);

int is_main(struct radio_state *radio, struct radio_state_sides *side);

void read_state_from_packet(struct display_packet *packet, struct radio_state *state);

#endif //RT8900_SERIAL_CONTROL_DISPLAY_PACKET_H