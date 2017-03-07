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
int dial_number(struct control_packet *base_packet, int number)
{
        const struct button_transmit_value *button = button_from_int(number);
        set_button(base_packet, button);

        return 0;
}