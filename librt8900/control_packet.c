#include "control_packet.h"

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