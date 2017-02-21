//
// Created by cormac on 16/02/17.
//
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include <stdio.h>
#include <pthread.h>

#include "control_packet.h"

void print_char(char byte)
{
        int i;
        for (i = 0; i < 8; i++) {
                printf("%d", ((byte<< i) & 0x80) != 0);
        }
        printf("\n");
}

void* start_send_packet(void* packet_pointer)
{
        //cast our void pointer back to a pointer pointer to the scruct
        CONTROL_PACKET** packet_ptr = (CONTROL_PACKET**)packet_pointer;

        while (1) {
                CONTROL_PACKET_INDEXED packet_arr = {.as_struct = **packet_ptr};
                int i;
                for (i = 0; i < sizeof(packet_arr.as_array); i++) {
                        print_char(packet_arr.as_array[i].raw);
                }
        #ifdef _WIN32
                Sleep(MILLISECONDS_BETWEEN_PACKETS);
        #else
                 usleep(MILLISECONDS_BETWEEN_PACKETS*1000);  /* sleep 3 milliSeconds */
        #endif
        }
}

int main() {
        pthread_t packet_send_thread;
        CONTROL_PACKET packet = control_packet_defaults;
        CONTROL_PACKET* packet_ptr = &packet;

        //cast our pointer pointer to void pointer for thred creation
        pthread_create(&packet_send_thread, NULL, start_send_packet, (void*)&packet_ptr);

        usleep(5000*1000);

        CONTROL_PACKET new_packet = control_packet_defaults;
        new_packet.ptt.section.data = 0x00;

        packet_ptr = &new_packet;

        pthread_join(packet_send_thread, NULL);

        return 0;
}