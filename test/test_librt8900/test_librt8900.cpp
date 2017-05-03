//
// Created by cormac on 25/04/17.
//

#include "test_librt8900.h"
extern "C" {
        #include "librt8900.h"
}

TEST(Librt8900Test, test_in_freq_range)
{
        set_log_level(RT8900_ERROR);

        EXPECT_TRUE(in_freq_range(145500) == VALID_FREQUENCY);
        EXPECT_TRUE(in_freq_range(985000) == VALID_FREQUENCY_RX_ONLY);

        EXPECT_TRUE(in_freq_range(0) == INVALID_FREQUENCY);
        EXPECT_TRUE(in_freq_range(-999999) == INVALID_FREQUENCY);
        EXPECT_TRUE(in_freq_range(9999999) == INVALID_FREQUENCY);

        set_log_level(RT8900_INFO);
}

TEST(Librt8900Test, test_current_freq_valid)
{
        set_log_level(RT8900_ERROR);

        //regular use case
        struct radio_side valid_radio = {0, 1455000, POWER_UNKNOWEN};
        EXPECT_TRUE(current_freq_valid(&valid_radio));

        //rx only should be false
        struct radio_side invalid_radio = {0, 8000000, POWER_UNKNOWEN};
        EXPECT_FALSE(current_freq_valid(&invalid_radio));

        struct radio_side invalid_radio1 = {0,0,POWER_UNKNOWEN};
        EXPECT_FALSE(current_freq_valid(&invalid_radio1));

        set_log_level(RT8900_INFO);
}