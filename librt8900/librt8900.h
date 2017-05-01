/**
 * @file librt8900.h
 * \brief The main headder file for the librt8900 libary.
 * Contains functions that use both the contol_packet and DISPLAY_PACKET
 */

#ifndef RT8900_SERIAL_CONTROL_API_H
#define RT8900_SERIAL_CONTROL_API_H

#include "log.h"
#include "serial.h"
#include "control_packet.h"
#include "display_packet.h"

#define VALID_POWER_LEVEL(num) ((num) == POWER_LOW || (num) == POWER_MEDIUM_FUZZY || (num) == POWER_HIGH)

/// Represents the abilities of the radio for a particular frequency
enum frequency_permission {
    INVALID_FREQUENCY = 0,
    VALID_FREQUENCY_RX_ONLY = 1,
    VALID_FREQUENCY = 2,
};

/// Represents one of the capable ranges of the radio
struct range_KHz {
    const bool tx_allowed;
    const char *name;
    const int low;
    const int high;
};

/// Configuration for sending packets
struct control_packet_sender_config {
    bool lazy_sending;
    bool dtr_pin_for_on;
    pthread_barrier_t* initialised;
    struct CONTROL_PACKET_Q_HEAD *queue;
    bool keep_alive;
};

/// Configuration for receiving packets
struct display_packet_receiving_config {
    bool keep_alive;
    bool radio_seen;
    pthread_mutex_t raw_packet_lock;
    unsigned char latest_raw_packet[DISPLAY_PACKET_SIZE];
};

typedef struct {
    char *serial_path;
    int serial_fd;
    bool shutdown_on_timeout;

    struct control_packet_sender_config send;
    struct display_packet_receiving_config receive;

} SERIAL_CFG;

//runner threads
void* send_control_packets(void *c);
void* receive_display_packets(void *c);

//valid frequency checks
const struct range_KHz * get_range(int frequency_khz);
int in_freq_range(int frequency_khz);

//helper functions
int get_frequency(struct radio_side *radio);
bool current_freq_valid(struct radio_side *radio);
void send_new_packet(SERIAL_CFG *config, struct control_packet *new_packet, enum pop_queue_behaviour free_choice);
bool check_radio_rx(SERIAL_CFG *config);
void wait_to_send(const SERIAL_CFG *cfg);
void shutdown_threads(SERIAL_CFG *cfg);

//settters
int set_frequency(SERIAL_CFG *cfg, struct control_packet *base_packet, int number);
int set_main_radio(SERIAL_CFG *cfg, struct control_packet *base_packet, enum radios side);
int set_left_power_level(SERIAL_CFG *cfg, struct control_packet *base_packet, enum rt8900_power_level power_level);
int set_right_power_level(SERIAL_CFG *cfg, struct control_packet *base_packet, enum rt8900_power_level power_level);
int set_power_button(SERIAL_CFG *cfg);

//getters
int get_display_packet(SERIAL_CFG *config, DISPLAY_PACKET packet);

//check other header files for more functions

#endif //RT8900_SERIAL_CONTROL_API_H
