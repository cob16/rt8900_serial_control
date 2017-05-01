#include "log.h"

/// \brief sets the global logging level of lib rt8900
void set_log_level(enum rt8900_logging_level i)
{
        rt8900_verbose_level = i;
}

/// \brief Used in place of println for logging
/// @param level is the log level this message should appear at
void log_msg(enum rt8900_logging_level level, char const *fmt, ...)
{
        if (rt8900_verbose_level >= level) {
                va_list args;
                va_start(args, fmt);

                vprintf(fmt, args);

                va_end(args);
        }
}