//
// Created by cormac on 14/03/17.
//
#include <unistd.h>

#include "display_packet.h"
#include "log.h"
#include "serial.h"

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

void read_packet(SERIAL_CFG *config, struct control_packet *packet)
{
        unsigned char buffer[DISPLAY_PACKET_SIZE];

        //todo check that there is a buffer so that we do not block forever

        int recived_bytes = read(config->serial_fd, &buffer, DISPLAY_PACKET_SIZE);

        int start_of_packet_index = find_packet_start(buffer, sizeof(buffer));

        if (recived_bytes != DISPLAY_PACKET_SIZE || start_of_packet_index == -1 ) {
                log_msg(RT8900_ERROR, "PACKET was corrupt / unrecognised! ");
                return;
        }


}
