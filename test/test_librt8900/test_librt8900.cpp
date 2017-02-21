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

TEST(ControlPacketTest, CONTROL_PACKET) {

        //create our object
        CONTROL_PACKET test_packet = control_packet_defaults;
        //test we can acess our data though the struct
        ASSERT_EQ(test_packet.ptt.section.data, 0x7F);

        //place our struct into a union that lets us get it as an array
        CONTROL_PACKET_INDEXED packet_arr = {.as_struct = test_packet};

        char controll_packet_expected_output[13] = {
                0x80,
                0x00,
                0x7f,
                0x00,
                0x1f,
                0x7f,
                0x00,
                0x00,
                0x7f,
                0x7f,
                0x7f,
                0x00,
                0x00
        };

        //test to see we have the same array size
        ASSERT_EQ(sizeof(packet_arr.as_array), sizeof(controll_packet_expected_output));

        //test the output with the expected output
        int i;
        for (i = 0; i < sizeof(controll_packet_expected_output); i++) {
                EXPECT_EQ(packet_arr.as_array[i].raw, controll_packet_expected_output[i]);
        }
}