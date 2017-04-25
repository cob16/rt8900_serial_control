//
// Created by cormac on 14/03/17.
//

#include <stddef.h>

#include "test_display_packet.h"
extern "C" {
        #include "librt8900.h"
}

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
        DISPLAY_PACKET ordered_packet = {};

        insert_shifted_packet(ordered_packet, messy_packet, 10, 5);

        EXPECT_EQ(ordered_packet[0].raw, messy_packet[5]);
        EXPECT_EQ(ordered_packet[1].raw, messy_packet[6]);
        EXPECT_EQ(ordered_packet[2].raw, messy_packet[7]);
        EXPECT_EQ(ordered_packet[3].raw, messy_packet[8]);
        EXPECT_EQ(ordered_packet[4].raw, messy_packet[9]);

        EXPECT_EQ(ordered_packet[5].raw, messy_packet[0]);
        EXPECT_EQ(ordered_packet[6].raw, messy_packet[1]);
        EXPECT_EQ(ordered_packet[7].raw, messy_packet[2]);
        EXPECT_EQ(ordered_packet[8].raw, messy_packet[3]);
        EXPECT_EQ(ordered_packet[9].raw, messy_packet[4]);
}

TEST(TestDisplayPacket, test_get_range)
{
        const struct range_KHz *range;

        // out of range
        range = get_range(0);
        EXPECT_TRUE(range == NULL);

        range = get_range(9999999);
        EXPECT_TRUE(range == NULL);

        //low edge range
        range = get_range(145000);
        ASSERT_FALSE(range == NULL);
        EXPECT_EQ(range->tx_allowed, true);
        EXPECT_STREQ(range->name, "2m Tx l");

        //in range
        range = get_range(145555);
        ASSERT_FALSE(range == NULL);
        EXPECT_EQ(range->tx_allowed, true);
        EXPECT_STREQ(range->name, "2m Tx l");

        //high edge range
        range = get_range(146000);
        ASSERT_FALSE(range == NULL);
        EXPECT_EQ(range->tx_allowed, true);
        EXPECT_STREQ(range->name, "2m Tx l");

        //same for rx only ranges
        range = get_range(980000);
        ASSERT_FALSE(range == NULL);
        EXPECT_EQ(range->tx_allowed, false);
        EXPECT_STREQ(range->name, "70cm");

        range = get_range(108000);
        ASSERT_FALSE(range == NULL);
        EXPECT_EQ(range->tx_allowed, false);
        EXPECT_STREQ(range->name, "2m RX");
}

TEST(TestDisplayPacket, test_operational_range)
{
        //out of range
        EXPECT_EQ(in_freq_range(0), 0);
        EXPECT_EQ(in_freq_range(9999999), 0);

        //rx only ranges
        EXPECT_EQ(in_freq_range(980000), 1);
        EXPECT_EQ(in_freq_range(108000), 1);

        //tx and rx ranges
        EXPECT_EQ(in_freq_range(145000), 2);
        EXPECT_EQ(in_freq_range(146000), 2);
}

void TestDisplayPacketReaders::SetUp()
{
        packet[0].section.check_num |= 1;
        packet[12].section.data |= 1 << 2; //left buisy
        packet[28].section.data |= 1 << 2; //right buizy
}

TEST_F(TestDisplayPacketReaders, test_read_busy)
{
        struct radio_state state;

        packet[12].section.data &= ~(1 << 2);
        packet[28].section.data &= ~(1 << 2);
        read_busy(packet, &state);

        EXPECT_EQ(state.left.busy, 0);
        EXPECT_EQ(state.right.busy, 0);

        //check reverce
        packet[12].section.data |= 1 << 2; //left buisy
        packet[28].section.data |= 1 << 2; //right buizy
        read_busy(packet, &state);

        EXPECT_EQ(state.left.busy, 1);
        EXPECT_EQ(state.right.busy, 1);

        EXPECT_EQ(display_packet_read(packet, LEFT_BUSY), 1);
        EXPECT_EQ(display_packet_read(packet, RIGHT_BUSY), 1);
}

TEST_F(TestDisplayPacketReaders, test_packet_read)
{
        EXPECT_EQ(display_packet_read(packet, LEFT_BUSY), 1);
        EXPECT_EQ(display_packet_read(packet, RIGHT_BUSY), 1);
}

TEST_F(TestDisplayPacketReaders, test_read_14_seg)
{
        //blank screen section == the number 0 as front 0's are hidden on the screen
        EXPECT_EQ(segment_to_int(0), 0);
        EXPECT_EQ(segment_to_int(0x1717), 0);

        EXPECT_EQ(segment_to_int(0x1400), 1);


}