#include "gtest/gtest.h"

#include "control_packet.h"

//used for debuging tests
void print_char(char byte)
{
        int i;
        for (i = 0; i < 8; i++) {
                printf("%d", ((byte<< i) & 0x80) != 0);
        }
        printf("\n");
}

TEST(ControlPacketTest, PACKET_BYTE)
{
        PACKET_BYTE test_byte = {.raw = 0x84};

        EXPECT_EQ(test_byte.section.check_num, 0x01);
        EXPECT_EQ(test_byte.section.data, 0x04);
}

TEST(ControlPacketTest, control_packet) {

        // C++ CAN'T HANDLE THIS CODE :/

//        CONTROL_PACKET packet;
//        packet = control_packet_defaults;
//        packet.volume_control_left.section.data = DATA_MIN_NUM;

//        CONTROL_PACKET_INDEXED packet_arr = {.as_array = packet};
//
//        int i;
//        for (i = 0; i < 12; i++) {
//                print_char(packet_arr.as_array[i].raw);
//        }
}