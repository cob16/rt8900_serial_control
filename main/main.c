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
#include <control_packet.h>

#include "control_packet.c"

void print_char(char byte)
{
        int i;
        for (i = 0; i < 8; i++) {
                printf("%d", ((byte<< i) & 0x80) != 0);
        }
        printf("\n");
}

static bool keep_alive = true;

void* start_send_packet(void* packet_pointer)
{
        //cast our void pointer back to a pointer pointer to the scruct
        CONTROL_PACKET** packet_ptr = (CONTROL_PACKET**)packet_pointer;
        CONTROL_PACKET* last_ptr;
        printf("--------------------------\n");
        printf("-- STARTING CONTROLL PACKET THREAD %p\n", packet_ptr);
        printf("-- pointer value %p\n", packet_ptr);
        printf("-- points to value %p\n", *packet_ptr);

        while (keep_alive) {
                CONTROL_PACKET *packet = *packet_ptr;
                if (packet != last_ptr) {
                        CONTROL_PACKET_INDEXED packet_arr = {.as_struct = *packet};
                        int i;
                        printf("--------------------------\n");
                        printf("SERIAL CONTROLL THREAD\nnew pointer address: %p \nNow sending:\n", packet);
                        for (i = 0; i < sizeof(packet_arr.as_array); i++) {
                                print_char(packet_arr.as_array[i].raw);
                        }
                        printf("--------------------------\n");
                        #ifdef _WIN32
                        Sleep(MILLISECONDS_BETWEEN_PACKETS);
                        #else
                        usleep(MILLISECONDS_BETWEEN_PACKETS * 1000);  /* sleep 3 milliSeconds */
                        #endif
                        last_ptr = packet;
                }
        }
}

int main() {
        pthread_t packet_send_thread;

        CONTROL_PACKET* packet = make_packet();
        CONTROL_PACKET **packet_ptr_ptr = &packet;

        CONFIG c = {
                .packet_pp = packet_ptr_ptr,
                .keep_alive = true
        };

        printf("pointer value %p\n", c.packet_pp);
        CONTROL_PACKET *cp = *c.packet_pp;
        printf("points to value %p\n", cp);
        print_char(cp->volume_control_right.raw);

        //cast our pointer pointer to void pointer for thred creation
        pthread_create(&packet_send_thread, NULL, start_send_packet, (void*)c.packet_pp);

//        CONTROL_PACKET *new_packet = make_packet();
        CONTROL_PACKET *new_packet = malloc(sizeof(CONTROL_PACKET));
        memcpy(new_packet, &control_packet_defaults,sizeof(CONTROL_PACKET));
        new_packet->ptt.section.data = 0x00;

        usleep(1000 * 1000);

        send_new_packet(&c, new_packet);

        keep_alive = false; //TODO HORABLE GLOBAL KILL ME
        pthread_join(packet_send_thread, NULL);

        return 0;
}