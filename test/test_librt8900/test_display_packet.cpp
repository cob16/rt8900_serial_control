//
// Created by cormac on 14/03/17.
//

#include "gtest/gtest.h"
#include "display_packet.c"

TEST(TestDisplayPacket, PACKET_BYTE)
{
        unsigned char packet[3] = {
                0x00,
                0x80,
                0x00,
        };
        EXPECT_EQ(find_packet_start(packet, 3), 1);

        packet[1] =  0x00;
        EXPECT_EQ(find_packet_start(packet, 3), -1);
}
