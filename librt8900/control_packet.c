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

/////Creates a config struct, you must supply an inital CONTROL_PACKET.
//SERIAL_CFG make_config(CONTROL_PACKET *packet){
//        CONTROL_PACKET **packet_ptr_ptr = &packet;
//        CONFIG conf = {
//                .packet = packet_ptr_ptr,
//                .keep_alive = true
//        };
//        return conf;
//}

///changes the pointer that the thread follows to send the packet AND FREES THE OLD ONE
void send_new_packet(SERIAL_CFG *config, CONTROL_PACKET *new_packet) {
        CONTROL_PACKET **cfg_pointer = config->packet;
        CONTROL_PACKET *active_packet = *cfg_pointer;

        //save the old pointer to free latter
        CONTROL_PACKET *oldpacket = active_packet;
        //do the swap
        *cfg_pointer = new_packet;

        free(oldpacket);
        oldpacket = NULL;

//        printf("Changed to new packet and freed to %p\n", new_packet);
}