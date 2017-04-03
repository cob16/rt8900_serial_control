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

enum display_packet_bitmasks {
        //Misc
        AUTO_POWER_OFF = BIT_LOCATED_AT(0, 1),
        KEYPAD_LOCK    = BIT_LOCATED_AT(30, 4),
        SET            = BIT_LOCATED_AT(4, 0),
        BD8600         = BIT_LOCATED_AT(4, 0),

        //Left side
        LEFT_BUISY = BIT_LOCATED_AT(12,2),
        LEFT_MAIN  = BIT_LOCATED_AT(36, 2),
        LEFT_TX    = BIT_LOCATED_AT(36, 3),

        LEFT_MINUS = BIT_LOCATED_AT(36, 5),
        LEFT_PLUSS = BIT_LOCATED_AT(36, 4),
        LEFT_DCS   = BIT_LOCATED_AT(9, 3),
        LEFT_DEC   = BIT_LOCATED_AT(37, 0),
        LEFT_ENC   = BIT_LOCATED_AT(36, 6),
        LEFT_SKIP  = BIT_LOCATED_AT(37, 2),
        LEFT_PMS   = BIT_LOCATED_AT(37, 1),

        LEFT_POWER_LOW    = BIT_LOCATED_AT(1, 5),
        LEFT_POWER_MEDIUM = BIT_LOCATED_AT(0, 2),

        //left 14 segment display
        LEFT_FREQ_1_A = BIT_LOCATED_AT(13, 0),
        LEFT_FREQ_1_B = BIT_LOCATED_AT(13, 1),
        LEFT_FREQ_1_C = BIT_LOCATED_AT(13, 2),
        LEFT_FREQ_1_D = BIT_LOCATED_AT(13, 3),
        LEFT_FREQ_1_E = BIT_LOCATED_AT(13, 4),
        LEFT_FREQ_1_F = BIT_LOCATED_AT(13, 5),
        LEFT_FREQ_1_G = BIT_LOCATED_AT(13, 6),
        LEFT_FREQ_1_H = BIT_LOCATED_AT(14, 1),
        LEFT_FREQ_1_I = BIT_LOCATED_AT(14, 2),
        LEFT_FREQ_1_J = BIT_LOCATED_AT(14, 3),
        LEFT_FREQ_1_K = BIT_LOCATED_AT(14, 5),
        LEFT_FREQ_1_L = BIT_LOCATED_AT(14, 6),
        LEFT_FREQ_1_M = BIT_LOCATED_AT(15, 0),

        LEFT_FREQ_2_A = BIT_LOCATED_AT(10, 5),
        LEFT_FREQ_2_B = BIT_LOCATED_AT(10, 6),
        LEFT_FREQ_2_C = BIT_LOCATED_AT(11, 0),
        LEFT_FREQ_2_D = BIT_LOCATED_AT(11, 1),
        LEFT_FREQ_2_E = BIT_LOCATED_AT(11, 2),
        LEFT_FREQ_2_F = BIT_LOCATED_AT(11, 3),
        LEFT_FREQ_2_G = BIT_LOCATED_AT(11, 4),
        LEFT_FREQ_2_H = BIT_LOCATED_AT(11, 6),
        LEFT_FREQ_2_I = BIT_LOCATED_AT(12, 0),
        LEFT_FREQ_2_J = BIT_LOCATED_AT(12, 1),
        LEFT_FREQ_2_K = BIT_LOCATED_AT(12, 3),
        LEFT_FREQ_2_L = BIT_LOCATED_AT(12, 4),
        LEFT_FREQ_2_M = BIT_LOCATED_AT(12, 5),

        //right side
        RIGHT_BUISY = BIT_LOCATED_AT(28,2),
        RIGHT_MAIN  = BIT_LOCATED_AT(32, 0),
        RIGHT_TX    = BIT_LOCATED_AT(32, 1),

        RIGHT_MINUS = BIT_LOCATED_AT(32, 3),
        RIGHT_PLUSS = BIT_LOCATED_AT(32, 2),
        RIGHT_DCS   = BIT_LOCATED_AT(22, 2),
        RIGHT_DEC   = BIT_LOCATED_AT(32, 5),
        RIGHT_ENC   = BIT_LOCATED_AT(32, 4),
        RIGHT_SKIP  = BIT_LOCATED_AT(33, 0),
        RIGHT_PMS   = BIT_LOCATED_AT(32, 6),

        RIGHT_POWER_LOW    = BIT_LOCATED_AT(17, 5),
        RIGHT_POWER_MEDIUM = BIT_LOCATED_AT(16, 2)
};

struct display_packet {
        PACKET_BYTE arr[42];
};

enum rt8900_power_level {
        POWER_UNKNOWEN = 0,
        POWER_LOW,
        POWER_MEDIUM_FUZZY,
        POWER_MEDIUM_1,
        POWER_MEDIUM_2,
        POWER_HIGH,
};

struct radio_state_sides {
        int busy;
        int frequency;
        enum rt8900_power_level power_level;
};


struct radio_state {
        struct radio_state_sides *main; //the currently selected pointer
        struct radio_state_sides left;
        struct radio_state_sides right;
};

int display_packet_read(struct display_packet *packet, const enum display_packet_bitmasks bit_number);
void insert_shifted_packet(struct display_packet *packet, unsigned char buffer[], size_t buffer_length, int start_of_packet_index);

void read_busy(struct display_packet *packet, struct radio_state *state);
void read_main(struct display_packet *packet, struct radio_state *state);
void read_power_fuzzy(struct display_packet *packet, struct radio_state *state);
void read_frequency(struct display_packet *packet, struct radio_state *state);

int decode_13_segment(int segment_bitmask);
int is_main(struct radio_state *radio, struct radio_state_sides *side);
void read_state_from_packet(struct display_packet *packet, struct radio_state *state);

#endif //RT8900_SERIAL_CONTROL_DISPLAY_PACKET_H