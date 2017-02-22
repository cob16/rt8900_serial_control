#include "control_packet.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CONTROL_PACKET * make_packet() {
        size_t ctrl_size = sizeof(CONTROL_PACKET);
        CONTROL_PACKET *packet = malloc(ctrl_size);
        memcpy(packet,&control_packet_defaults,sizeof(ctrl_size));
        return packet;
}

///Creates a config struct, you must supply an inital CONTROL_PACKET.
CONFIG make_config(CONTROL_PACKET *packet){
        CONTROL_PACKET **packet_ptr_ptr = &packet;
        CONFIG conf = {
                .packet_pp = packet_ptr_ptr,
                .keep_alive = true
        };
        return conf;
}

///points the thred to the new packet and frees the old one
void send_new_packet(CONFIG *config, CONTROL_PACKET *new_packet) {
        CONTROL_PACKET **cfg_pointer = config->packet_pp;
        CONTROL_PACKET *active_packet = *cfg_pointer;

        printf("+ active_pointer is  %p\n", cfg_pointer);
        printf("+ points to  %p\n", active_packet);

        CONTROL_PACKET *oldpacket = active_packet;

        *cfg_pointer = new_packet;
        printf("+ changed to %p\n", new_packet);

        printf("+ about to free %p\n", oldpacket);
        free(oldpacket);
        oldpacket = NULL;


}