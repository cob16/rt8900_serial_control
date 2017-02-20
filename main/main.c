//
// Created by cormac on 16/02/17.
//

#include <stdio.h>
#include <control_packet.h>

void print_char(char byte)
{
        int i;
        for (i = 0; i < 8; i++) {
                printf("%d", ((byte<< i) & 0x80) != 0);
        }
        printf("\n");
}

int main() {
        CONTROL_PACKET packet;
        packet = control_packet_defaults;
        packet.volume_control_left.section.data = DATA_MIN_NUM;

        print_char(packet.ptt.raw);
        print_char(packet.encoder_right.raw);
        print_char(packet.keypad_input_row.raw);
        return 0;
}