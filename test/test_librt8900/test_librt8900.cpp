#include <packet.h>
#include "gtest/gtest.h"

#include "control_packet.c"

//used for debuging tests
//void print_char(char byte)
//{
//        int i;
//        for (i = 0; i < 8; i++) {
//                printf("%d", ((byte<< i) & 0x80) != 0);
//        }
//        printf("\n");
//}

TEST(ControlPacketTest, PACKET_BYTE)
{
        PACKET_BYTE test_byte = {.raw = 0x84};

        EXPECT_EQ(test_byte.section.check_num, 0x01);
        EXPECT_EQ(test_byte.section.data, 0x04);
}

TEST(ControlPacketTest, moretest) {
        int test = 1;
        EXPECT_EQ(example_function(&test), &test);
}

TEST(ControlPacketTest, evenmoretest) {
        int test = 1;
        EXPECT_EQ(example_function(&test), &test);
}