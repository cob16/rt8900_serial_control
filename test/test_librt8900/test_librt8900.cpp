#include <packet.h>
#include "gtest/gtest.h"
#include "control_packet.h"

//used for debugging tests
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
        PACKET_BYTE test_byte = {.raw = (signed char) 0x84};

        EXPECT_EQ(test_byte.section.check_num, 0x01);
        EXPECT_EQ(test_byte.section.data, 0x04);
}

TEST(ControlPacketTest, CONTROL_PACKET) {

        //create our object
        CONTROL_PACKET test_packet = control_packet_defaults;
        //test we can access our data though the struct
        ASSERT_EQ(test_packet.ptt.section.data, 0x7F);

        //place our struct into a union that lets us get it as an array
        CONTROL_PACKET_INDEXED packet_arr = {.as_struct = test_packet};

        signed char packet_expected_output[13] = {
                (signed char) 0x80, //encoder_right
                (signed char) 0x00, //encoder_left
                (signed char) 0x7f, //ptt
                (signed char) 0x00, //squelch_right
                (signed char) 0x1f, //volume_control_right
                (signed char) 0x7f, //keypad_input_row
                (signed char) 0x00, //squelch_left
                (signed char) 0x1f, //volume_control_left
                (signed char) 0x7f, //keypad_input_column
                (signed char) 0x7f, //panel_buttons_right
                (signed char) 0x7f, //panel_buttons_left
                (signed char) 0x00, //menu_buttons
                (signed char) 0x00  //hyper_mem_buttons
        };

        //test to see we have the same array size
        ASSERT_EQ(sizeof(packet_arr.as_array), sizeof(packet_expected_output));

        //test the output with the expected output
        int i;
        for (i = 0; i < sizeof(packet_expected_output); i++) {
//                printf("testing %x == %x at packet byte #%d \n",
//                       packet_arr.as_array[i].raw,
//                       packet_expected_output[i],
//                       i);
                ASSERT_EQ(packet_arr.as_array[i].raw, packet_expected_output[i]);
        }
}