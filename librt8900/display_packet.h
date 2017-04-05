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

        //left 13 segment display
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

        LEFT_FREQ_3_A = BIT_LOCATED_AT(8, 3),
        LEFT_FREQ_3_B = BIT_LOCATED_AT(8, 4),
        LEFT_FREQ_3_C = BIT_LOCATED_AT(8, 5),
        LEFT_FREQ_3_D = BIT_LOCATED_AT(8, 6),
        LEFT_FREQ_3_E = BIT_LOCATED_AT(9, 0),
        LEFT_FREQ_3_F = BIT_LOCATED_AT(9, 1),
        LEFT_FREQ_3_G = BIT_LOCATED_AT(9, 2),
        LEFT_FREQ_3_H = BIT_LOCATED_AT(9, 4),
        LEFT_FREQ_3_I = BIT_LOCATED_AT(9, 5),
        LEFT_FREQ_3_J = BIT_LOCATED_AT(9, 6),
        LEFT_FREQ_3_K = BIT_LOCATED_AT(10, 1),
        LEFT_FREQ_3_L = BIT_LOCATED_AT(10, 2),
        LEFT_FREQ_3_M = BIT_LOCATED_AT(10, 3),

        LEFT_FREQ_4_A = BIT_LOCATED_AT(5, 2),
        LEFT_FREQ_4_B = BIT_LOCATED_AT(5, 3),
        LEFT_FREQ_4_C = BIT_LOCATED_AT(5, 4),
        LEFT_FREQ_4_D = BIT_LOCATED_AT(5, 5),
        LEFT_FREQ_4_E = BIT_LOCATED_AT(5, 6),
        LEFT_FREQ_4_F = BIT_LOCATED_AT(6, 0),
        LEFT_FREQ_4_G = BIT_LOCATED_AT(6, 1),
        LEFT_FREQ_4_H = BIT_LOCATED_AT(6, 3),
        LEFT_FREQ_4_I = BIT_LOCATED_AT(6, 4),
        LEFT_FREQ_4_J = BIT_LOCATED_AT(6, 5),
        LEFT_FREQ_4_K = BIT_LOCATED_AT(7, 0),
        LEFT_FREQ_4_L = BIT_LOCATED_AT(7, 1),
        LEFT_FREQ_4_M = BIT_LOCATED_AT(7, 2),

        LEFT_FREQ_5_A = BIT_LOCATED_AT(3, 0),
        LEFT_FREQ_5_B = BIT_LOCATED_AT(3, 1),
        LEFT_FREQ_5_C = BIT_LOCATED_AT(3, 2),
        LEFT_FREQ_5_D = BIT_LOCATED_AT(3, 3),
        LEFT_FREQ_5_E = BIT_LOCATED_AT(3, 4),
        LEFT_FREQ_5_F = BIT_LOCATED_AT(3, 5),
        LEFT_FREQ_5_G = BIT_LOCATED_AT(3, 6),
        LEFT_FREQ_5_H = BIT_LOCATED_AT(4, 1),
        LEFT_FREQ_5_I = BIT_LOCATED_AT(4, 2),
        LEFT_FREQ_5_J = BIT_LOCATED_AT(4, 3),
        LEFT_FREQ_5_K = BIT_LOCATED_AT(4, 5),
        LEFT_FREQ_5_L = BIT_LOCATED_AT(4, 6),
        LEFT_FREQ_5_M = BIT_LOCATED_AT(5, 0),

        LEFT_FREQ_6_A = BIT_LOCATED_AT(0, 5),
        LEFT_FREQ_6_B = BIT_LOCATED_AT(0, 6),
        LEFT_FREQ_6_C = BIT_LOCATED_AT(1, 0),
        LEFT_FREQ_6_D = BIT_LOCATED_AT(1, 1),
        LEFT_FREQ_6_E = BIT_LOCATED_AT(1, 2),
        LEFT_FREQ_6_F = BIT_LOCATED_AT(1, 3),
        LEFT_FREQ_6_G = BIT_LOCATED_AT(1, 4),
        LEFT_FREQ_6_H = BIT_LOCATED_AT(1, 6),
        LEFT_FREQ_6_I = BIT_LOCATED_AT(2, 0),
        LEFT_FREQ_6_J = BIT_LOCATED_AT(2, 1),
        LEFT_FREQ_6_K = BIT_LOCATED_AT(2, 3),
        LEFT_FREQ_6_L = BIT_LOCATED_AT(2, 4),
        LEFT_FREQ_6_M = BIT_LOCATED_AT(2, 5),

        LEFT_FREQ_7   = 0,

        LEFT_FREQ_PEROID = BIT_LOCATED_AT(0, 3),

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
        RIGHT_POWER_MEDIUM = BIT_LOCATED_AT(16, 2),

        //right 13 segment display
        RIGHT_FREQ_1_A = BIT_LOCATED_AT(29, 0),
        RIGHT_FREQ_1_B = BIT_LOCATED_AT(29, 1),
        RIGHT_FREQ_1_C = BIT_LOCATED_AT(29, 2),
        RIGHT_FREQ_1_D = BIT_LOCATED_AT(29, 3),
        RIGHT_FREQ_1_E = BIT_LOCATED_AT(29, 4),
        RIGHT_FREQ_1_F = BIT_LOCATED_AT(29, 5),
        RIGHT_FREQ_1_G = BIT_LOCATED_AT(29, 6),
        RIGHT_FREQ_1_H = BIT_LOCATED_AT(30, 1),
        RIGHT_FREQ_1_I = BIT_LOCATED_AT(30, 1),
        RIGHT_FREQ_1_J = BIT_LOCATED_AT(30, 3),
        RIGHT_FREQ_1_K = BIT_LOCATED_AT(30, 5),
        RIGHT_FREQ_1_L = BIT_LOCATED_AT(30, 6),
        RIGHT_FREQ_1_M = BIT_LOCATED_AT(31, 0),

        RIGHT_FREQ_2_A = BIT_LOCATED_AT(26, 5),
        RIGHT_FREQ_2_B = BIT_LOCATED_AT(26, 6),
        RIGHT_FREQ_2_C = BIT_LOCATED_AT(27, 0),
        RIGHT_FREQ_2_D = BIT_LOCATED_AT(27, 1),
        RIGHT_FREQ_2_E = BIT_LOCATED_AT(27, 2),
        RIGHT_FREQ_2_F = BIT_LOCATED_AT(27, 3),
        RIGHT_FREQ_2_G = BIT_LOCATED_AT(27, 4),
        RIGHT_FREQ_2_H = BIT_LOCATED_AT(27, 6),
        RIGHT_FREQ_2_I = BIT_LOCATED_AT(28, 0),
        RIGHT_FREQ_2_J = BIT_LOCATED_AT(28, 1),
        RIGHT_FREQ_2_K = BIT_LOCATED_AT(28, 3),
        RIGHT_FREQ_2_L = BIT_LOCATED_AT(28, 4),
        RIGHT_FREQ_2_M = BIT_LOCATED_AT(28, 5),

        RIGHT_FREQ_3_A = BIT_LOCATED_AT(24, 3),
        RIGHT_FREQ_3_B = BIT_LOCATED_AT(24, 4),
        RIGHT_FREQ_3_C = BIT_LOCATED_AT(24, 5),
        RIGHT_FREQ_3_D = BIT_LOCATED_AT(24, 6),
        RIGHT_FREQ_3_E = BIT_LOCATED_AT(25, 0),
        RIGHT_FREQ_3_F = BIT_LOCATED_AT(25, 1),
        RIGHT_FREQ_3_G = BIT_LOCATED_AT(25, 2),
        RIGHT_FREQ_3_H = BIT_LOCATED_AT(25, 4),
        RIGHT_FREQ_3_I = BIT_LOCATED_AT(25, 5),
        RIGHT_FREQ_3_J = BIT_LOCATED_AT(25, 6),
        RIGHT_FREQ_3_K = BIT_LOCATED_AT(26, 1),
        RIGHT_FREQ_3_L = BIT_LOCATED_AT(26, 2),
        RIGHT_FREQ_3_M = BIT_LOCATED_AT(26, 3),

        RIGHT_FREQ_4_A = BIT_LOCATED_AT(21, 2),
        RIGHT_FREQ_4_B = BIT_LOCATED_AT(21, 3),
        RIGHT_FREQ_4_C = BIT_LOCATED_AT(21, 4),
        RIGHT_FREQ_4_D = BIT_LOCATED_AT(21, 5),
        RIGHT_FREQ_4_E = BIT_LOCATED_AT(21, 6),
        RIGHT_FREQ_4_F = BIT_LOCATED_AT(22, 0),
        RIGHT_FREQ_4_G = BIT_LOCATED_AT(22, 1),
        RIGHT_FREQ_4_H = BIT_LOCATED_AT(22, 3),
        RIGHT_FREQ_4_I = BIT_LOCATED_AT(22, 4),
        RIGHT_FREQ_4_J = BIT_LOCATED_AT(22, 5),
        RIGHT_FREQ_4_K = BIT_LOCATED_AT(23, 0),
        RIGHT_FREQ_4_L = BIT_LOCATED_AT(23, 1),
        RIGHT_FREQ_4_M = BIT_LOCATED_AT(23, 2),

        RIGHT_FREQ_5_A = BIT_LOCATED_AT(19, 0),
        RIGHT_FREQ_5_B = BIT_LOCATED_AT(19, 1),
        RIGHT_FREQ_5_C = BIT_LOCATED_AT(19, 2),
        RIGHT_FREQ_5_D = BIT_LOCATED_AT(19, 3),
        RIGHT_FREQ_5_E = BIT_LOCATED_AT(19, 4),
        RIGHT_FREQ_5_F = BIT_LOCATED_AT(19, 5),
        RIGHT_FREQ_5_G = BIT_LOCATED_AT(19, 6),
        RIGHT_FREQ_5_H = BIT_LOCATED_AT(20, 1),
        RIGHT_FREQ_5_I = BIT_LOCATED_AT(20, 2),
        RIGHT_FREQ_5_J = BIT_LOCATED_AT(20, 3),
        RIGHT_FREQ_5_K = BIT_LOCATED_AT(20, 5),
        RIGHT_FREQ_5_L = BIT_LOCATED_AT(20, 6),
        RIGHT_FREQ_5_M = BIT_LOCATED_AT(21, 0),

        RIGHT_FREQ_6_A = BIT_LOCATED_AT(16, 5),
        RIGHT_FREQ_6_B = BIT_LOCATED_AT(16, 6),
        RIGHT_FREQ_6_C = BIT_LOCATED_AT(17, 0),
        RIGHT_FREQ_6_D = BIT_LOCATED_AT(17, 1),
        RIGHT_FREQ_6_E = BIT_LOCATED_AT(17, 2),
        RIGHT_FREQ_6_F = BIT_LOCATED_AT(17, 3),
        RIGHT_FREQ_6_G = BIT_LOCATED_AT(17, 4),
        RIGHT_FREQ_6_H = BIT_LOCATED_AT(17, 6),
        RIGHT_FREQ_6_I = BIT_LOCATED_AT(18, 0),
        RIGHT_FREQ_6_J = BIT_LOCATED_AT(18, 1),
        RIGHT_FREQ_6_K = BIT_LOCATED_AT(18, 3),
        RIGHT_FREQ_6_L = BIT_LOCATED_AT(18, 4),
        RIGHT_FREQ_6_M = BIT_LOCATED_AT(18, 5),

        RIGHT_FREQ_7 = BIT_LOCATED_AT(16, 0),
        RIGHT_FREQ_PEROID = BIT_LOCATED_AT(16, 3),
};

typedef PACKET_BYTE DISPLAY_PACKET[DISPLAY_PACKET_SIZE];

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

int display_packet_read(DISPLAY_PACKET packet, const enum display_packet_bitmasks bit_number);
void insert_shifted_packet(DISPLAY_PACKET packet, unsigned char buffer[], size_t buffer_length, int start_of_packet_index);
int segment_to_int(int segment_bitmask);

void read_busy(DISPLAY_PACKET packet, struct radio_state *state);
void read_main(DISPLAY_PACKET packet, struct radio_state *state);
void read_power_fuzzy(DISPLAY_PACKET packet, struct radio_state *state);
int read_frequency(DISPLAY_PACKET packet, struct radio_state *state);

int is_main(struct radio_state *radio, struct radio_state_sides *side);
void read_state_from_packet(DISPLAY_PACKET packet, struct radio_state *state);

#endif //RT8900_SERIAL_CONTROL_DISPLAY_PACKET_H