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

/// the start of the known packet could be anywhere in the buffer
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

///Check that we are reciving from teh radio.
int check_radio_rx(int fd)
{
        int current_buffer_bytes;
        int read = ioctl(fd, FIONREAD, &current_buffer_bytes);
        log_msg(RT8900_DEBUG, "There are %d bytes available to read in the buffer\n", current_buffer_bytes);
        if (current_buffer_bytes < 42 || read == -1) {
                log_msg(RT8900_ERROR, "NO DATA RECEIVED! \n Please make sure that the radio is connected and/or turned on\n");
                return 1;
        }
        return 0;
}

int get_display_packet(SERIAL_CFG *config, struct display_packet *packet)
{
        tcflush(config->serial_fd, TCIFLUSH); //flush the buffer to get the latest data
        usleep(MS_PACKET_WAIT_TIME * 1000); //enough time for at least 1 packet to be sent

        if (check_radio_rx(config->serial_fd) != 0) {
                return 1;
        }

        unsigned char buffer[DISPLAY_PACKET_SIZE];

        int recived_bytes = read(config->serial_fd, &buffer, DISPLAY_PACKET_SIZE);
        int start_of_packet_index = find_packet_start(buffer, sizeof(buffer));

        if (recived_bytes != DISPLAY_PACKET_SIZE || start_of_packet_index == -1 ) {
                log_msg(RT8900_ERROR, "PACKET was corrupt / unrecognised!\n");
                return 1;
        }
        insert_shifted_packet(packet, buffer, DISPLAY_PACKET_SIZE, start_of_packet_index);
        return 0;
}

///Gets busy state from display_packet
void read_busy(struct display_packet *packet, struct radio_state *state)
{
        state->left.busy = (1 & packet->arr[12].raw >> 2);
        state->right.busy = (1 & packet->arr[28].raw >> 2);
}

///Sets the main correct pointer to the correct radio, NULL if nether selected
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

int is_main(struct radio_state *radio, struct radio_state_sides *side)
{
        return (radio->main == side);
}


void read_state_from_packet(struct display_packet *packet, struct radio_state *state) {

        read_busy(packet, state);
        read_main(packet, state);
};

int get_state(SERIAL_CFG *config, struct radio_state *state) {
        struct display_packet packet;
        get_display_packet(config, &packet);
        read_state_from_packet(&packet, state);
        return 1;
}
