//
// Created by cormac on 14/03/17.
//

#ifndef RT8900_SERIAL_CONTROL_DISPLAY_PACKET_H
#define RT8900_SERIAL_CONTROL_DISPLAY_PACKET_H

//number of bytes in a display packet
#define DISPLAY_PACKET_SIZE 42

#define MS_PACKET_WAIT_TIME 25 //time for a full packet cycle

#include "packet.h"
#include "serial.h"

struct display_packet {
        PACKET_BYTE arr[42];
};

enum power_level {
        low = 1,
        medium = 2,
        medium2 = 3,
        high = 4
};

struct radio_state_sides {
        int busy;
        char frequency;
        enum power_level power_level;
};


struct radio_state {
        struct radio_state_sides *main; //the currently selected pointer
        struct radio_state_sides left;
        struct radio_state_sides right;
};

struct control_packet_config {
    bool lazy_sending;
    pthread_barrier_t* initialised;
    struct CONTROL_PACKET_Q_HEAD *queue;
    bool keep_alive;
};

struct display_packet_config {
        //todo remove if this is not usefull
};

void insert_shifted_packet(struct display_packet *packet, unsigned char buffer[], size_t buffer_length, int start_of_packet_index);
int check_radio_rx(int fd);
void read_busy(struct display_packet *packet, struct radio_state *state);
void read_main(struct display_packet *packet, struct radio_state *state);
int is_main(struct radio_state *radio, struct radio_state_sides *side);

void read_state_from_packet(struct display_packet *packet, struct radio_state *state);

#endif //RT8900_SERIAL_CONTROL_DISPLAY_PACKET_H