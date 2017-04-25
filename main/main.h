//
// Created by cormac on 16/02/17.
//

#ifndef RT8900_SERIAL_CONTROL_MAIN_H
#define RT8900_SERIAL_CONTROL_MAIN_H

#include "librt8900.h"

#define PROMPT_BUFFER_SIZE 32

#define TURN_ON_RADIO_TRYS 3

#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"

// Define our user_commands here so that we can organise our shell better
int cmd_help(char **args, SERIAL_CFG *config, struct control_packet *base_packet);
int cmd_exit(char **args, SERIAL_CFG *config, struct control_packet *base_packet);

int cmd_get_frequency(char **args, SERIAL_CFG *config, struct control_packet *base_packet);
int cmd_set_frequency(char **args, SERIAL_CFG *config, struct control_packet *base_packet);

int cmd_get_busy(char **args, SERIAL_CFG *config, struct control_packet *base_packet);

int cmd_get_main(char **args, SERIAL_CFG *config, struct control_packet *base_packet);
int cmd_set_main(char **args, SERIAL_CFG *config, struct control_packet *base_packet);

int cmd_get_power(char **args, SERIAL_CFG *config, struct control_packet *base_packet);
int cmd_set_power(char **args, SERIAL_CFG *config, struct control_packet *base_packet);

int cmd_get_ptt(char **args, SERIAL_CFG *config, struct control_packet *base_packet);
int cmd_set_ptt(char **args, SERIAL_CFG *config, struct control_packet *base_packet);

int cmd_set_volume(char **args, SERIAL_CFG *config, struct control_packet *base_packet);
int cmd_set_squelch(char **args, SERIAL_CFG *config, struct control_packet *base_packet);



#endif //RT8900_SERIAL_CONTROLL_MAIN_H