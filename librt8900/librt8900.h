//
// Created by cormac on 25/03/17.
//

#ifndef RT8900_SERIAL_CONTROL_API_H
#define RT8900_SERIAL_CONTROL_API_H
//
#include "log.h"
#include "serial.h"
#include "control_packet.h"
#include "display_packet.h"

struct control_packet_sender_config {
    bool lazy_sending;
    pthread_barrier_t* initialised;
    struct CONTROL_PACKET_Q_HEAD *queue;
    bool keep_alive;
};

typedef struct {
    char *serial_path;
    int serial_fd;

    struct control_packet_sender_config send;

} SERIAL_CFG;

//todo all 'public' functions should be defined here and return int (or perhaps enum)

//internal functions
void send_new_packet(SERIAL_CFG *config, struct control_packet *new_packet, enum pop_queue_behaviour free_choice);
void* send_control_packets(void *c);
int get_display_packet(SERIAL_CFG *config, struct display_packet *packet);

//settters
int set_frequency(SERIAL_CFG *cfg, struct control_packet *base_packet, int number);
int set_main_radio(SERIAL_CFG *cfg, struct control_packet *base_packet, enum radios side);

//getters

#endif //RT8900_SERIAL_CONTROL_API_H
