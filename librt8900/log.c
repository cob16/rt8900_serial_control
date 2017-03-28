#include "log.h"

void set_log_level(enum rt8900_logging_level i)
{
        rt8900_verbose_level = i;
}

void log_msg(enum rt8900_logging_level level, char const *fmt, ...)
{

        va_list args;
        va_start(args, fmt);

        if (rt8900_verbose_level >= level) {
                vprintf(fmt, args);
        }
        va_end(args);
}