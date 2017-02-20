//
// Created by cormac on 20/02/17.
//

#ifndef RT8900_SERIAL_CONTROLL_PACKET_H
#define RT8900_SERIAL_CONTROLL_PACKET_H

#endif //RT8900_SERIAL_CONTROLL_PACKET_H

#pragma pack(1) //we don't want space between our bits
typedef struct
{
    unsigned int data: 7;  // 6 more bits of the byte
    unsigned int check_num: 1;  // the last bit int he byte (1st in packet is 1 else 0)
} FT8900BYTE;

typedef union
{
    FT8900BYTE section;
    char raw;
} PACKET_BYTE;