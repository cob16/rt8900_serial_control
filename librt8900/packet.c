#include <stdio.h>
#include "log.h"

void print_char(char byte)
{
        int i;
        for (i = 0; i < 8; i++) {
                printf("%d", ((byte<< i) & 0x80) != 0);
        }
        printf("\n");
}

/// the start of the known packet could be anywhere in the buffer
/// this function finds the starting index based of it's bit marker
int find_packet_start(unsigned char buffer[], size_t length) {
        int i;
        for (i = 0; i < length; i++) {
                if ( (1 & buffer[i] >> 7) ) { //is the 8th bit set to 1
                        return i;
                };
        }
        return -1;
}