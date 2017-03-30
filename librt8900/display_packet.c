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
//truth table for a 14 segment display
/*      A       B       C       D       E       F       G       H       I       J       K       L       M       hex
 * 0    1	1	1	1	1	1	0	0	1	0	1	1	1       1F97
 * 1    1	0	1	0	0	0	0	0	0	0	0	0	0       1400
 * 2    0	1	1	0	1	1	0	0	1	0	0	1	0       D92
 * 3
 * 4
 * 5
 * 6
 * 7
 * 8
 * 9
 * macros from truth table */
//todo convert to hex else we are stuck to gcc
#define FOURTEEN_SEG_0 0b1111110010111
#define FOURTEEN_SEG_1 0b1010000000000

///Takes an char of bits (ordered as described bellow) ofa decodes and returns the number
int decode_14_segment(int segment_bitmask)
{
        switch (segment_bitmask) {
        case FOURTEEN_SEG_0:
                return 0;
        case FOURTEEN_SEG_1:
                return 1;
        default:
                return -1;
        }
}

void contanate_bit(int *result, char bit)
{
        *result = (*result << 1) | bit;
}

int display_packet_read(struct display_packet *packet, int bit_number)
{
        int bit = bit_number % 8;
        int byte = bit_number / 8;

        return (packet->arr[byte].raw >> bit) & 0x80 != 0;
};

void read_left_frequency(struct display_packet *packet, struct radio_state *state)
{
        if (1 & (packet->arr[36].raw >> 2)) {
                state->main = &state->left;
        } else if (1 & packet->arr[32].raw) {
                state->main = &(state->right);
        } else {
                log_msg(RT8900_ERROR, "Could not get selected radio 'main' from PACKET\n");
                state->main = NULL;
        }
}

int is_main(struct radio_state *radio, struct radio_state_sides *side)
{
        return (radio->main == side);
}


void read_state_from_packet(struct display_packet *packet, struct radio_state *state) {

        read_busy(packet, state);
        read_main(packet, state);
};
