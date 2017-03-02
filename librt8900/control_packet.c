//
// Created by cormac on 01/03/17.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "control_packet.h"


void set_button(struct control_packet *packet, const struct button_transmit_value *button)
{
        packet->keypad_input_row.section.data    = button->row;
        packet->keypad_input_column.section.data = button->column;
}

///Creates the required packet to dial a number. they should then be added to the head of the queue*
int dial_number(struct control_packet *base_packet, int number) //TODO FINISH THIS
{
        int num_digets = snprintf(NULL, 0, "%d", number);
        if (num_digets > 6){
                //this number
                printf("WARNING!: dialing a %d digit number! (%d) (Only 6 required for frequency inputs)", num_digets, number);
        }
        char digits[num_digets];
        snprintf(digits, sizeof(num_digets), "%d", number);

        int i;
        for (i=0; i<num_digets; i++){
                struct control_packet *packet = (struct control_packet*) malloc(sizeof(*packet));
                memcpy(packet, &control_packet_defaults,sizeof(*packet));

                const struct button_transmit_value *button = button_from_int(digits[i] - '0');
                set_button(packet, button);
        }

        return 0;
}