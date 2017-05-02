//
// Created by cormac on 25/03/17.
//

#include "librt8900.h"
#include "serial.h"

#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>

#define NUMBER_TX_BANDS 4
const struct range_KHz AVALABLE_TX_BANDS[NUMBER_TX_BANDS] = {
        {true, "10m"    , 28000, 29700},
        {true, "6m"     , 50000, 54000},
        {true, "2m Tx l", 144000, 146000},
        {true, "2m Tx h", 340000, 440000},
};

#define NUMBER_RX_BANDS 2
const struct range_KHz AVAILABLE_RX_BANDS[NUMBER_RX_BANDS] = {
        {false, "2m RX", 108000, 180000},
        {false, "70cm",  700000, 985000},
};

/// Gets the current frequency of the radio
int get_frequency(struct radio_side *radio) {
        /* frequency is represented as is always 7 digits, however the last digit
         * is not permitted to be entered with the keypad so we do not use it here */
        return radio->frequency / 10;
}


/*! Returns (struct range_KHz) if input is within that range. Else returns NULL
 * Looks for tx&rx ranges first */
const struct range_KHz * get_range(int frequency_khz)
{
        int i;
        for (i = 0; i < NUMBER_TX_BANDS; i++){
                if (AVALABLE_TX_BANDS[i].low <= frequency_khz && frequency_khz <= AVALABLE_TX_BANDS[i].high) {
                        return &AVALABLE_TX_BANDS[i];
                }
        }
        for (i = 0; i < NUMBER_RX_BANDS; i++){
                if (AVAILABLE_RX_BANDS[i].low <= frequency_khz && frequency_khz <= AVAILABLE_RX_BANDS[i].high) {
                        return &AVAILABLE_RX_BANDS[i];
                }
        }
        return NULL;
}

/*! returns 0 if invalid range, * or 1 if only rx allowed, * and 2 for all allowed */
int in_freq_range(int frequency_khz)
{
        const struct range_KHz *range = get_range(frequency_khz);
        if (range != NULL) {
                if (range->tx_allowed) {
                        return VALID_FREQUENCY;
                } else {
                        log_msg(RT8900_INFO, "Tx is not allowed on this frequency (%d) \n", frequency_khz);
                        return VALID_FREQUENCY_RX_ONLY;
                }
        }
        return INVALID_FREQUENCY;
}

/*! Adds a control_packet (pointer) to the send queue,
 * should only be called once the queue has been initialized*/
void send_new_packet(SERIAL_CFG *config, struct control_packet *new_packet, enum pop_queue_behaviour free_choice)
{
        if (new_packet == NULL) {
                log_msg(RT8900_ERROR, "NULL packet attempted to be added to QUEUE\n");
        } else {
                struct control_packet_q_node *node = (struct control_packet_q_node*) malloc(sizeof(*(node)));
                node->packet = new_packet;
                node->free_packet = free_choice;

                TAILQ_INSERT_TAIL(config->send.queue, node, nodes);
                log_msg(RT8900_TRACE, "ADDDED TO QUEUE \n");
        }
}
/*! Starts sending control packets as defined by SERIAL_CFG
 *
 *  This function is designed to be started as a thread  */
void* send_control_packets(void *c)
{
        log_msg(RT8900_TRACE, "-- STARTING CONTROL PACKET THREAD\n");
        SERIAL_CFG *conf = (SERIAL_CFG*) c;
        open_serial(&(conf->serial_fd), &(conf->serial_path), &(conf->send.dtr_pin_for_on)); //setup our serial connection

        //setup queue
        CONTROL_PACKET_Q_HEAD head;
        TAILQ_INIT(&head);
        conf->send.queue = &head;
        conf->send.keep_alive = true;

        useconds_t us_between_packets;
        int repeat_packets;

        //sets where to emulate the head of the radio or just send a little as possible (without loss of speed)
        if (conf->send.lazy_sending) {
                us_between_packets = MILLISECONDS_DEBOUNCE_WAIT * 1000;
                repeat_packets = 1;
        } else {
                us_between_packets = MILLISECONDS_BETWEEN_PACKETS_STANDARD * 1000;
                repeat_packets = 8;
        }

        struct control_packet_q_node *current_node = NULL;
        struct control_packet *current_packet = NULL;
        struct control_packet *last_packet = NULL;
        int packets_sent = 0;

        pthread_barrier_wait(conf->send.initialised);
        while (conf->send.keep_alive) {
                if  (!TAILQ_EMPTY(&head)) {
                        current_node = TAILQ_FIRST(conf->send.queue);
                        current_packet = current_node->packet;
                        CONTROL_PACKET_INDEXED packet_arr = {.as_struct = *current_packet};

                        //print debug
                        if (current_packet != last_packet) {
                                log_msg(RT8900_TRACE, "-\n");
                                if (rt8900_verbose_level >= RT8900_TRACE) {
                                        packet_debug(current_packet, &packet_arr);
                                }
                                last_packet = current_packet;
                                packets_sent = 0;
                        }

                        //SEND THE PACKET
                        do {
                                write(conf->serial_fd, packet_arr.as_array, sizeof(packet_arr.as_array));
                                tcdrain(conf->serial_fd); //wait for the packet to send
                                packets_sent++;
#ifdef _WIN32
                                Sleep(us_between_packets);
#else
                                usleep(us_between_packets);
#endif
                        } while (packets_sent < repeat_packets);

                        //move to the next packet and clean up
                        if (current_node->nodes.tqe_next != NULL) { //pop if there is more to send
                                log_msg(RT8900_TRACE, "removed after %d packets sent\n", packets_sent);
                                TAILQ_REMOVE(conf->send.queue, current_node, nodes);
                                if (current_node->free_packet == PACKET_FREE_AFTER_SEND) {
                                        free(current_packet);
                                }
                                packets_sent = 0;
                                current_packet = NULL;
                                free(current_node);
                                current_node = NULL;
                        }
                } else {
                        log_msg(RT8900_TRACE, "Send queue is empty!\n");
                }


        }

        //clean out our queue incase of shutdown
        while (!TAILQ_EMPTY(conf->send.queue)) {
                current_node = TAILQ_FIRST(conf->send.queue);
                current_packet = current_node->packet;
                TAILQ_REMOVE(conf->send.queue, current_node, nodes);
                free(current_packet);
                current_packet = NULL;
        }

        return NULL;
}

/*! Writes the latest packet to a segment of memory
 *  This function is designed to be started as a thread  */
void* receive_display_packets(void *c)
{
        SERIAL_CFG *config = (SERIAL_CFG*) c;
        config->receive.keep_alive = true;

        // allows our mutex to be locked across threads
        pthread_mutexattr_t shared;
        pthread_mutexattr_init(&shared);
        pthread_mutexattr_setpshared(&shared, PTHREAD_PROCESS_SHARED);

        if (pthread_mutex_init(&(config->receive.raw_packet_lock), &shared) != 0)
        {
                log_msg(RT8900_ERROR, "mutex init failed\n");
                exit(EXIT_FAILURE);
        }

        //open serial if not already open
        if (config->serial_fd  < 1) {
                //setup our serial connection if not already done
                open_serial(&(config->serial_fd), &(config->serial_path), &(config->send.dtr_pin_for_on));
        }

        tcflush(config->serial_fd, TCIFLUSH); //flush any existing buffer

        while (config->receive.keep_alive) {
                usleep(MS_PACKET_WAIT_TIME * 1000); //enough time for at least 1 packet to be sent

                // Initialize all the file descriptor sets
                fd_set read_fds, write_fds, except_fds;
                FD_ZERO(&read_fds);
                FD_ZERO(&write_fds);
                FD_ZERO(&except_fds);
                FD_SET(config->serial_fd, &read_fds);

                struct timeval timeout;
                timeout.tv_sec = 1;
                timeout.tv_usec = 0;

                // select wil return false if the timeout has been reached
                if (select(config->serial_fd + 1, &read_fds, &write_fds, &except_fds, &timeout) == 1) {
                        pthread_mutex_lock(&(config->receive.raw_packet_lock));
                        read(config->serial_fd, &(config->receive.latest_raw_packet), DISPLAY_PACKET_SIZE);
                        pthread_mutex_unlock(&(config->receive.raw_packet_lock));
                        config->receive.radio_seen = true;
                } else {
                        //Abort
                        log_msg(RT8900_FATAL, "NO DATA RECEIVED FROM RADIO !\n");
                        if (config->shutdown_on_timeout == true) {
                                log_msg(RT8900_FATAL, "NO DATA RECEIVED and shutdown_on_timeout is true!\n");
                                shutdown_threads(config);
                        }
                }
        }

        pthread_mutex_destroy(&(config->receive.raw_packet_lock));

        return 0;
}

/*! \brief writes the most recent packet to @param packet.
 *  Gets the most recent packet. Will block if there is currently a half updated most recent packet */
int get_display_packet(SERIAL_CFG *config, DISPLAY_PACKET packet)
{
        unsigned char temp_buffer[DISPLAY_PACKET_SIZE];
        pthread_mutex_lock(&(config->receive.raw_packet_lock));
        memcpy(&temp_buffer, (config->receive.latest_raw_packet) , sizeof(temp_buffer));
        pthread_mutex_unlock(&(config->receive.raw_packet_lock));


        int start_of_packet_index = find_packet_start(temp_buffer, sizeof(temp_buffer));
        if (start_of_packet_index == -1 ) {
                log_msg(RT8900_ERROR, "PACKET was corrupt / unrecognised!\n");
                return 0;
        }
        insert_shifted_packet(packet, temp_buffer, DISPLAY_PACKET_SIZE, start_of_packet_index);
        return 1;
}

/*! \brief Check that we are received from the radio at lest once.
 * @warning Will still return True if the radio is disconnected after some time.
 * @returns true is the radio has been seen else false
 * */
bool check_radio_rx(SERIAL_CFG *config)
{
        return config->receive.radio_seen;
}

/*! \brief Switches context to the desired radio
 *
 * First check you are not already on this mode using is_main() else you will enter frequency edit mode! */
int set_main_radio(SERIAL_CFG *cfg, struct control_packet *base_packet, enum radios side) {
        maloc_control_packet(switch_main);
        memcpy(switch_main, base_packet, sizeof(*switch_main));

        switch (side){
        case RADIO_LEFT:
                set_main_button(switch_main, L_ENCODER_BUTTON);
                break;
        case RADIO_RIGHT:
                set_main_button(switch_main, R_ENCODER_BUTTON);
                break;
        default:
                free(switch_main);
                return -1;
        }

        send_new_packet(cfg, switch_main, PACKET_FREE_AFTER_SEND);
        send_new_packet(cfg, base_packet, PACKET_ONLY_SEND);
        return 0;

}

/*! \brief Adds the required packets to dial a number.
 *
 * They are then be added to the queue
 * @returns 0 on success */
int set_frequency(SERIAL_CFG *cfg, struct control_packet *base_packet, int number)
{
        if (!in_freq_range(number)) {
                log_msg(RT8900_ERROR, "%d is not a frequency that can be set on this radio", number);
                return 1;
        }
        log_msg(RT8900_INFO, "dialing %d\n", number);

        //get the number of digits
        int num_digits = snprintf(NULL, 0, "%d", number);

        //create a char array of the digits
        char digits[num_digits];
        snprintf(digits, num_digits + 1, "%d", number);

        //We assume smaller numbers are missing front 0's so we dial them here
        int i;
        for (i=0; i<(6 - num_digits); i++) {
                maloc_control_packet(dialnum);
                memcpy(dialnum, base_packet, sizeof(*base_packet));
                set_keypad_button(dialnum, button_from_int(0));
                send_new_packet(cfg, dialnum, PACKET_FREE_AFTER_SEND);
                send_new_packet(cfg, base_packet, PACKET_ONLY_SEND);
                log_msg(RT8900_DEBUG, "dialing 0\n");
        }

        //dial the remaining numbers
        for (i=0; i<num_digits; i++){
                maloc_control_packet(dialnum);
                memcpy(dialnum, base_packet, sizeof(*base_packet));
                set_keypad_button(dialnum, button_from_int(digits[i] - '0'));
                send_new_packet(cfg, dialnum, PACKET_FREE_AFTER_SEND);
                send_new_packet(cfg, base_packet, PACKET_ONLY_SEND);
                log_msg(RT8900_DEBUG, "dialing %d\n", digits[i] - '0');
        }
        return 0;
}

/*! \brief gracefully stops send_control_packets() and receive_display_packets()
 *
 *  This can be used anytime to gracefully stop sending and receiving on serial
 *  Threads will be able to join after running ths function */
void shutdown_threads(SERIAL_CFG *cfg)
{
        cfg->send.keep_alive = false;
        cfg->receive.keep_alive = false;
        if (cfg->serial_fd != 0) {
                tcflush(cfg->serial_fd, TCIOFLUSH); //flush in/out buffers
                close(cfg->serial_fd);
        }
}

/*! \brief Blocks until there are no new packets to send */
void wait_to_send(const SERIAL_CFG *cfg)
{
        if (cfg->send.keep_alive == true && !TAILQ_EMPTY(cfg->send.queue)) {
                while (cfg->send.queue->tqh_first->nodes.tqe_next != NULL) {
                        usleep(MILLISECONDS_DEBOUNCE_WAIT * 1000);
                };
                usleep(MILLISECONDS_DEBOUNCE_WAIT * 1000);
        }
}

/*! @return True if the radio is currently set to a frequency it can TX on */
bool current_freq_valid(struct radio_side *radio)
{
        return (in_freq_range(get_frequency(radio)) == VALID_FREQUENCY);
}

/*! \brief sets left power level on radio
 *
 * Presses the Low button on the radio until the selected power is set*/
int set_left_power_level(SERIAL_CFG *cfg, struct control_packet *base_packet, enum rt8900_power_level power_level)
{
        //todo when diget reading is completed this will need to be updated to allow selection of med1 and med1 levels

        if (!VALID_POWER_LEVEL(power_level)) {
                return 1;
        }

        DISPLAY_PACKET packet;
        if (get_display_packet(cfg, packet) != 1) {
                log_msg(RT8900_ERROR, "failed to get DISPLAY_PACKET");
                return 1;
        };

        struct radio_state state;
        read_power_fuzzy(packet, &state);
        read_frequency(packet, &state);

        //Power can only be set in the radio if on a frequency that supports Tx
        if (!(current_freq_valid(&(state.left)))) {
                log_msg(RT8900_WARNING, "The current left frequency does not permit TX.\nTherefore changing the radio power is also not permitted.\n");
                return 2;
        }

        while(state.left.power_level != power_level && cfg->send.keep_alive == true) {
                maloc_control_packet(power_press);
                memcpy(power_press, base_packet, sizeof(*base_packet));
                set_left_button(power_press, LEFT_LOW);

                send_new_packet(cfg, power_press, PACKET_FREE_AFTER_SEND);
                send_new_packet(cfg, base_packet, PACKET_ONLY_SEND);
                wait_to_send(cfg);

                sleep(2); //the radio is very inconstant on change time this is here for some safty
                if (get_display_packet(cfg, packet) != 1) {
                        log_msg(RT8900_ERROR, "failed to get DISPLAY_PACKET");
                        break;
                };
                read_power_fuzzy(packet, &state);
        }

        return 0;
}

/*! \brief sets right power level on radio
 *
 * Presses the Low button on the radio until the selected power is set
 *
 * @returns 0 on sucess, 1 on internal error and 2 on user error
 * */
int set_right_power_level(SERIAL_CFG *cfg, struct control_packet *base_packet, enum rt8900_power_level power_level)
{
        //todo when diget reading is completed this will need to be updated to allow selection of med1 and med1 levels

        if (!VALID_POWER_LEVEL(power_level)) {
                return 2;
        }

        DISPLAY_PACKET packet;
        if (get_display_packet(cfg, packet) != 1) {
                log_msg(RT8900_ERROR, "failed to get DISPLAY_PACKET");
                return 1;
        };

        struct radio_state state;
        read_power_fuzzy(packet, &state);
        read_frequency(packet, &state);

        //Power can only be set in the radio if on a frequency that supports Tx
        if (!(current_freq_valid(&(state.right)))) {
                log_msg(RT8900_WARNING, "The current right frequency does not permit TX.\nTherefore changing the radio power is also not permitted.\n");
                return 2;
        }

        while(state.right.power_level != power_level && cfg->send.keep_alive == true) {
                maloc_control_packet(power_press);
                memcpy(power_press, base_packet, sizeof(*base_packet));
                set_right_button(power_press, RIGHT_LOW);

                send_new_packet(cfg, power_press, PACKET_FREE_AFTER_SEND);
                send_new_packet(cfg, base_packet, PACKET_ONLY_SEND);
                wait_to_send(cfg);

                /* We do not up now what the packet latency is between sending a cmd and getting a updated screen
                 * so we will wait a second to guarantee the program can se the update */
                // todo this could be reduced
                sleep(2);

                if (get_display_packet(cfg, packet) != 1) {
                        log_msg(RT8900_ERROR, "failed to get DISPLAY_PACKET");
                        return 1;
                };
                read_power_fuzzy(packet, &state);
        }

        return 0;
}

///Experimental! Sets the dtr pin low for one second to trigger radio on
int set_power_button(SERIAL_CFG *cfg)
{
        if (cfg->send.dtr_pin_for_on == false) {
                log_msg(RT8900_ERROR, "Power on flag not provided, please see help output to enable");
                return 1;
        }

        int RTS_flag = TIOCM_DTR;
        ioctl(cfg->serial_fd, TIOCMGET, &RTS_flag);
        RTS_flag &= ~TIOCM_DTR;
        ioctl(cfg->serial_fd, TIOCMSET, &RTS_flag);

        sleep(1);

        ioctl(cfg->serial_fd, TIOCMGET, &RTS_flag);
        RTS_flag |= TIOCM_DTR;
        ioctl(cfg->serial_fd, TIOCMSET, &RTS_flag);

        if (check_radio_rx(cfg) == false) {
                log_msg(RT8900_ERROR, "Failed to turn on radio\n");
                return 1;
        }

        return 0;
}