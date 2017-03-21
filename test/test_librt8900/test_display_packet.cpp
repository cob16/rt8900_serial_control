//
// Created by cormac on 14/03/17.
//

#include <packet.h>
#include "gtest/gtest.h"
#include "display_packet.c"
#include "test_display_packet.h"

TEST(TestDisplayPacket, test_find_packet_start)
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

TEST(TestDisplayPacket, test_shift_array)
{
        unsigned char messy_packet[10] = {
                0x05, 0x06, 0x07, 0x08, 0x09,
                0x80, 0x01, 0x02, 0x03, 0x04
        };
        struct display_packet ordered_packet = {.arr = {}};

        insert_shifted_packet(&ordered_packet, messy_packet, 10, 5);

        EXPECT_EQ(ordered_packet.arr[0].raw, messy_packet[5]);
        EXPECT_EQ(ordered_packet.arr[1].raw, messy_packet[6]);
        EXPECT_EQ(ordered_packet.arr[2].raw, messy_packet[7]);
        EXPECT_EQ(ordered_packet.arr[3].raw, messy_packet[8]);
        EXPECT_EQ(ordered_packet.arr[4].raw, messy_packet[9]);

        EXPECT_EQ(ordered_packet.arr[5].raw, messy_packet[0]);
        EXPECT_EQ(ordered_packet.arr[6].raw, messy_packet[1]);
        EXPECT_EQ(ordered_packet.arr[7].raw, messy_packet[2]);
        EXPECT_EQ(ordered_packet.arr[8].raw, messy_packet[3]);
        EXPECT_EQ(ordered_packet.arr[9].raw, messy_packet[4]);

}

void TestDisplayPacketReaders::SetUp()
{
        packet.arr[0].section.check_num = 0x01;

        packet.arr[28].raw = 0x00;
}

TEST_F(TestDisplayPacketReaders, test_read_busy)
{
        struct radio_state state;
        read_busy(&packet, &state);

        EXPECT_EQ(state.left.busy, 0);
        EXPECT_EQ(state.right.busy, 0);

        //check reverce
        packet.arr[12].section.data |= 1 << 2; //left buisy
        packet.arr[28].section.data |= 1 << 2; //right buizy
        read_busy(&packet, &state);

        EXPECT_EQ(state.left.busy, 1);
        EXPECT_EQ(state.right.busy, 1);
}