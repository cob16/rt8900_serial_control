/**
 * @file log.h
 * \brief logging wrapper with levels for the librt8900 library.
 *
 * Contains functions that use both the control_packet and DISPLAY_PACKET.
 */

#ifndef RT8900_LOG_H
#define RT8900_LOG_H

#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>

enum rt8900_logging_level {
    RT8900_NOLOG = 0,
    RT8900_FATAL,
    RT8900_ERROR,
    RT8900_WARNING,
    RT8900_INFO,
    RT8900_DEBUG,
    RT8900_TRACE,
};

/// The default logging level for the library
static enum rt8900_logging_level rt8900_verbose_level = RT8900_INFO;

void set_log_level(enum rt8900_logging_level i);
void log_msg(enum rt8900_logging_level level, char const *fmt, ...);

#endif //RT8900_LOG_H