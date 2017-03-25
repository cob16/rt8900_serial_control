//
// Created by cormac on 16/02/17.
//

#ifndef RT8900_SERIAL_CONTROL_MAIN_H
#define RT8900_SERIAL_CONTROL_MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <argp.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include "librt8900.h"

#define PROMPT_BUFFER_SIZE 32

#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#endif //RT8900_SERIAL_CONTROLL_MAIN_H