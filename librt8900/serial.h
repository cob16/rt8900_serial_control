/**
 * @file serial.h
 * \brief Serial handling.
 */

#ifndef RT8900_SERIAL_CONTROL_SERIAL_H
#define RT8900_SERIAL_CONTROL_SERIAL_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>

void open_serial(int *fd, char **serial_path, bool *dtr_pin_for_on);

#endif //RT8900_SERIAL_CONTROLL_SERIAL_H
