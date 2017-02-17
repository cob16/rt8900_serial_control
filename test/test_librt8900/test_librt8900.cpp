#include "control_packet.c"
#include "gtest/gtest.h"

TEST(ControlPacketTest, TestWeCanTest) {
    int test = 1;
    EXPECT_EQ(example_function(&test), &test);
}

TEST(ControlPacketTest, moretest) {
    int test = 1;
    EXPECT_EQ(example_function(&test), &test);
}

TEST(ControlPacketTest, evenmoretest) {
    int test = 1;
    EXPECT_EQ(example_function(&test), &test);
}