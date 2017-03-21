//
// Created by cormac on 14/03/17.
//
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

#include "display_packet.h"
#include "log.h"
#include "serial.h"
#include "packet.h"

/// the start of the known packet could be anywhre in the buffer
/// this function finds the starting index based of it's bit marker
int find_packet_start(unsigned char buffer[], size_t length) {
        int i;
        for (i = 0; i < length; i++) {
                if ( (1 & buffer[i] >> 7) ) { //is the 8th bit set to 1
                        return i;
                };
        }
        return -1;
}

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

int get_display_packet(SERIAL_CFG *config, struct display_packet *packet)
{
        unsigned char buffer[DISPLAY_PACKET_SIZE];

        //Check that there is a buffer so that we do not block forever
        int current_buffer_bytes;
        ioctl(config->serial_fd, FIONREAD, &current_buffer_bytes);
        if (current_buffer_bytes == 0) {
                log_msg(RT8900_ERROR, "NO BYTES HAVE BEEN RECEIVED!\n");
                return 1;
        }

        printf("There are %d bytes available to read in the buffer", current_buffer_bytes);

        int recived_bytes = read(config->serial_fd, &buffer, DISPLAY_PACKET_SIZE);
        int start_of_packet_index = find_packet_start(buffer, sizeof(buffer));

        if (recived_bytes != DISPLAY_PACKET_SIZE || start_of_packet_index == -1 ) {
                log_msg(RT8900_ERROR, "PACKET was corrupt / unrecognised!\n");
                return 1;
        }
        insert_shifted_packet(packet, buffer, DISPLAY_PACKET_SIZE, start_of_packet_index);
        return 0;
}

void read_busy(struct display_packet *packet, struct radio_state *state)
{
        state->left.busy = (1 & packet->arr[12].raw >> 2);
        state->right.busy = (1 & packet->arr[28].raw >> 2);
}

///sets the main correct pointer to the correct radio, NULL if nether selected
void read_main(struct display_packet *packet, struct radio_state *state)
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


void read_state_from_packet(struct display_packet *packet, struct radio_state *state) {

        read_busy(packet, state);
};

int get_state(SERIAL_CFG *config, struct radio_state *state) {

        struct display_packet packet;
        read_state_from_packet(&packet, state);
        return 1;
}
