//
// Created by cormac on 13/03/17.
//

#include "gtest/gtest.h"

#ifndef fobar
#define fobar
#include "control_packet.c"
#endif //this

TEST(TestAPISetters, test_safe_int_char)
{
        EXPECT_EQ(safe_int_char(1), (signed char) 1);
}