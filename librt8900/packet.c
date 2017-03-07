#include <stdio.h>
#include "rt8900_log.h"

void print_char(char byte)
{
        int i;
        for (i = 0; i < 8; i++) {
                log_msg(RT8900_TRACE, "%d", ((byte<< i) & 0x80) != 0);
        }
        log_msg(RT8900_TRACE, "\n");
}