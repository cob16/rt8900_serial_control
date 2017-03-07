#include <packet.h>
#include "gtest/gtest.h"

#include "test_librt8900.h"
#include "control_packet.c"

TEST(ControlPacketTest, PACKET_BYTE)
{
        PACKET_BYTE test_byte = {.raw = (signed char) 0x84};

        EXPECT_EQ(test_byte.section.check_num, 0x01);
        EXPECT_EQ(test_byte.section.data, 0x04);
}

TEST(ControlPacketTest, CONTROL_PACKET) {

        //create our object
        struct control_packet test_packet = control_packet_defaults;
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
                (signed char) 0x1f, //volume_control_left
                (signed char) 0x00, //squelch_left
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

TEST(TestKeypadButtons, test_set_button)
{
        //create a packet
        struct control_packet *press_a_button = (control_packet *) malloc(sizeof(*press_a_button));
        memcpy(press_a_button, &control_packet_defaults,sizeof(*press_a_button));

        //test the function sets correctly
        set_button(press_a_button, &BUTTON_1);
        EXPECT_EQ(press_a_button->keypad_input_row.section.data, BUTTON_1.row);
        EXPECT_EQ(press_a_button->keypad_input_column.section.data, BUTTON_1.column);

        //make sure we handled the constant right i.e we copied the value
        press_a_button->keypad_input_row.section.data = (signed char) 0X7F;
        EXPECT_EQ(press_a_button->keypad_input_row.section.data, BUTTON_NONE.row);
        EXPECT_EQ(BUTTON_1.row, 0x00);
}

TEST(TestKeypadButtons, test_button_from_int)
{
        //test invalid range
        const struct button_transmit_value *no_press = button_from_int(-1);
        EXPECT_EQ(no_press->column, BUTTON_NONE.column);
        EXPECT_EQ(no_press->row, BUTTON_NONE.row);

        no_press = button_from_int(10);
        EXPECT_EQ(no_press->column, BUTTON_NONE.column);
        EXPECT_EQ(no_press->row, BUTTON_NONE.row);

        //check the expected output
        const struct button_transmit_value *test_button_5 = button_from_int(5);
        EXPECT_EQ(test_button_5->row,    (signed char) 0X1A);
        EXPECT_EQ(test_button_5->column, (signed char) 0X32);
}
