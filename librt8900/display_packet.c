//
// Created by cormac on 14/03/17.
//
#include <unistd.h>
#include <sys/ioctl.h>

#include "display_packet.h"

/// Write to the packet in the correct order.
/// For example the packet may start at index 10. Assumes buffer array length is DISPLAY_PACKET_SIZE (42)
void insert_shifted_packet(struct display_packet *packet, unsigned char buffer[], size_t buffer_length, int start_of_packet_index)
{
        // Starting from index to the end
        int i = 0;
        int packet_index = 0;
        for (i = start_of_packet_index; i < buffer_length; i++, packet_index++) {
                packet->arr[packet_index].raw = buffer[i];
        }
        //now from the beginning to the index
        for (i = 0; i < start_of_packet_index; i++, packet_index++) { ;
                packet->arr[packet_index].raw = buffer[i];
        }
}

///Gets busy state from display_packet
void read_busy(struct display_packet *packet, struct radio_state *state)
{
        state->left.busy  = display_packet_read(packet, LEFT_BUISY);
        state->right.busy = display_packet_read(packet, RIGHT_BUISY);
}

///Sets the main correct pointer to the correct radio, NULL if nether selected
void read_main(struct display_packet *packet, struct radio_state *state)
{
        int left = display_packet_read(packet, LEFT_MAIN);
        int right = display_packet_read(packet, RIGHT_MAIN);

        if (left == right) {
                log_msg(RT8900_ERROR, "Could not determine 'main' radio  from PACKET\n");
                state->main = NULL;
        } else if (left) {
                state->main = &state->left;
        } else {
                state->main = &(state->right);
        }
}

/// Gets the power levels of the radios only using reads,
// todo Currently there is no way to know if med1 or med2 is set
// todo create a seprate non fussy function that will be called to toggel power button 4 times (a circle) and then read the screen
void read_power_fuzzy(struct display_packet *packet, struct radio_state *state)
{
        if (display_packet_read(packet, LEFT_POWER_LOW)) {
                state->left.power_level = POWER_LOW;
        } else if (display_packet_read(packet, LEFT_POWER_MEDIUM)) {
                state->left.power_level = POWER_MEDIUM_FUZZY;
        } else {
                state->left.power_level = POWER_HIGH;
        }

        if (display_packet_read(packet, RIGHT_POWER_LOW)) {
                state->right.power_level = POWER_LOW;
        } else if (display_packet_read(packet, RIGHT_POWER_MEDIUM)) {
                state->right.power_level = POWER_MEDIUM_FUZZY;
        } else {
                state->right.power_level = POWER_HIGH;
        }
}

//todo this
//truth table for a 13 segment display
/*      A       B       C       D       E       F       G       H       I       J       K       L       M       hex
 * 0    1	1	1	1	1	1	0	0	1	0	1	1	1       1F97
 * 1    1	0	1	0	0	0	0	0	0	0	0	0	0       1400
 * 2    0	1	1	0	1	1	0	0	1	0	0	1	0       D92
 * 3    1       1       1       0       1       1       0       0       1       0       0       0       0
 * 4    1       1       1       0       0       1       0       0       0       0       0       0       1
 * 5
 * 6
 * 7
 * 8
 * 9
 * macros from truth table */
//todo convert to hex else we are stuck to gcc
#define THIRTEEN_SEG_0 0b1111110010111
#define THIRTEEN_SEG_1 0b1010000000000
#define THIRTEEN_SEG_2 0b0110110010010
#define THIRTEEN_SEG_3 0b1110110010000
#define THIRTEEN_SEG_4 0b1110010000001

///Takes an char of bits (ordered as described bellow) ofa decodes and returns the number
int decode_13_segment(int segment_bitmask)
{
        switch (segment_bitmask) {
        case 0: //left digets that are 0 are not showen
        case THIRTEEN_SEG_0:
                return 0;
        case THIRTEEN_SEG_1:
                return 1;
        case THIRTEEN_SEG_2:
                return 2;
        case THIRTEEN_SEG_3:
                return 3;
        case THIRTEEN_SEG_4:
                return 4;
        default:
                return -1;
        }
}

void create_bit_field(int *result, char bit)
{
        *result = (*result << 1) | bit;
}

int display_packet_read(struct display_packet *packet, const enum display_packet_bitmasks bit_number)
{
        int bit = bit_number % 8;
        int byte = bit_number / 8;

        return (packet->arr[byte].raw >> bit) & 0x80 != 0;
};

void read_frequency(struct display_packet *packet, struct radio_state *state)
{

        state->left.frequency = 0;

        int first_diget[13] = {
                display_packet_read(packet, LEFT_FREQ_1_A),
                display_packet_read(packet, LEFT_FREQ_1_B),
                display_packet_read(packet, LEFT_FREQ_1_C),
                display_packet_read(packet, LEFT_FREQ_1_D),
                display_packet_read(packet, LEFT_FREQ_1_E),
                display_packet_read(packet, LEFT_FREQ_1_F),
                display_packet_read(packet, LEFT_FREQ_1_G),
                display_packet_read(packet, LEFT_FREQ_1_H),
                display_packet_read(packet, LEFT_FREQ_1_I),
                display_packet_read(packet, LEFT_FREQ_1_J),
                display_packet_read(packet, LEFT_FREQ_1_K),
                display_packet_read(packet, LEFT_FREQ_1_L),
                display_packet_read(packet, LEFT_FREQ_1_M)
        };

        int bit_field = 0;
        int i;
        for (i=0; i<13; i++) {
                printf("%d " , first_diget[i]);
                bit_field = (bit_field << 1) | first_diget[i];
        }
        printf(" -> %d\n", decode_13_segment(bit_field));

        state->left.frequency = decode_13_segment(bit_field) * 100000;

        int second_diget[13] = {
                display_packet_read(packet, LEFT_FREQ_2_A),
                display_packet_read(packet, LEFT_FREQ_2_B),
                display_packet_read(packet, LEFT_FREQ_2_C),
                display_packet_read(packet, LEFT_FREQ_2_D),
                display_packet_read(packet, LEFT_FREQ_2_E),
                display_packet_read(packet, LEFT_FREQ_2_F),
                display_packet_read(packet, LEFT_FREQ_2_G),
                display_packet_read(packet, LEFT_FREQ_2_H),
                display_packet_read(packet, LEFT_FREQ_2_I),
                display_packet_read(packet, LEFT_FREQ_2_J),
                display_packet_read(packet, LEFT_FREQ_2_K),
                display_packet_read(packet, LEFT_FREQ_2_L),
                display_packet_read(packet, LEFT_FREQ_2_M)
        };


        bit_field = 0;
        i = 0;
        for (i=0; i<13; i++) {
                printf("%d " , second_diget[i]);
                bit_field = (bit_field << 1) | second_diget[i];
        }
        printf(" -> %d\n", decode_13_segment(bit_field));

        state->left.frequency = state->left.frequency + (decode_13_segment(bit_field) * 10000);

        state->right.frequency = 000000;
}

int is_main(struct radio_state *radio, struct radio_state_sides *side)
{
        return (radio->main == side);
}


void read_state_from_packet(struct display_packet *packet, struct radio_state *state) {

        read_busy(packet, state);
        read_main(packet, state);
};
