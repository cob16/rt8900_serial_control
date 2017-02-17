//
// Created by cormac on 16/02/17.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "control_packet.h"

#define WELCOME "WELCOME to the thing or somthing"

int main() {
    int number = 12;
    printf("%d\n", number);
    set_to_3(&number);
    printf("%d\n", number);
    return 0;
}