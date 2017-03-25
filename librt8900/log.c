#include "log.h"

static enum rt8900_logging_level rt8900_verbose = RT8900_ERROR;

void log_msg(enum rt8900_logging_level level, char const *fmt, ...)
{
        va_list args;
        va_start(args, fmt);

        if (rt8900_verbose >= level) {
                vfprintf(stderr, fmt, args);
        }
        va_end(args);
}