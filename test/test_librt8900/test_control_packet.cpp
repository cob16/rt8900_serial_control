#include "gtest/gtest.h"

#include "test_control_packet.h"
#include "control_packet.c"

TEST(ControlPacketTest, PACKET_BYTE)
{
        PACKET_BYTE test_byte = {.raw = (signed char) 0x84};

        EXPECT_EQ(test_byte.section.check_num, 0x01);
        EXPECT_EQ(test_byte.section.data, 0x04);
}

TEST(ControlPacketTest, CONTROL_PACKET)
{

        //create our object
        struct control_packet test_packet = control_packet_defaults;
        //test we can access our data though the struct
        ASSERT_EQ(test_packet.ptt.section.data, 0x7F);

        //place our struct into a union that lets us get it as an array
        CONTROL_PACKET_INDEXED packet_arr = {.as_struct = test_packet};

        unsigned char packet_expected_output[13] = {
                (unsigned char) 0x80, //encoder_right
                (unsigned char) 0x00, //encoder_left
                (unsigned char) 0x7f, //ptt
                (unsigned char) 0x00, //squelch_right
                (unsigned char) 0x1f, //volume_control_right
                (unsigned char) 0x7f, //keypad_input_row
                (unsigned char) 0x1f, //volume_control_left
                (unsigned char) 0x00, //squelch_left
                (unsigned char) 0x7f, //keypad_input_column
                (unsigned char) 0x7f, //right_buttons
                (unsigned char) 0x7f, //left_buttons
                (unsigned char) 0x00, //main_buttons
                (unsigned char) 0x00  //hyper_mem_buttons
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
        set_keypad_button(press_a_button, &BUTTON_1);
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

TEST(TestAPISetters, test_safe_int_char)
{
        EXPECT_EQ(safe_int_char(-1), (signed char) 0);
        EXPECT_EQ(safe_int_char(128), (signed char) 127);

        EXPECT_EQ(safe_int_char(9999999), (signed char) 127);
        EXPECT_EQ(safe_int_char((int)NULL), (signed char) 0);

        EXPECT_EQ(safe_int_char(2), (signed char) 2);
}

TEST(TestAPISetters, test_set_L_R_volume)
{
        maloc_control_packet(packet)
        memcpy(packet, &control_packet_defaults ,sizeof(*packet));

        //set to 50
        EXPECT_EQ(set_volume_left(packet, 50) , 0);
        EXPECT_EQ(set_volume_right(packet, 50), 0);
        EXPECT_EQ(packet->volume_control_left.section.data, (signed char) 50);
        EXPECT_EQ(packet->volume_control_right.section.data, (signed char) 50);

        //set to -100 (invalid should set to 0)
        EXPECT_EQ(set_volume_left(packet, -100) , 0);
        EXPECT_EQ(set_volume_right(packet, -100), 0);
        EXPECT_EQ(packet->volume_control_left.section.data, (signed char) 0);
        EXPECT_EQ(packet->volume_control_right.section.data, (signed char) 0);

        //test we properly handle a null pointer
        free(packet);
        packet = NULL;
        ASSERT_NO_THROW(set_volume_right(NULL, 666));
        ASSERT_NO_THROW(set_volume_left(NULL, 666));
        ASSERT_EQ(set_volume_right(NULL, 666), 1);
        ASSERT_EQ(set_volume_left(NULL, 666), 1);
}

TEST(TestAPISetters, test_set_L_R_squelch)
{
        maloc_control_packet(packet)
        memcpy(packet, &control_packet_defaults ,sizeof(*packet));

        //set to 50
        EXPECT_EQ(set_squelch_left(packet, 127) , 0);
        EXPECT_EQ(set_squelch_right(packet, 127), 0);
        EXPECT_EQ(packet->squelch_left.section.data, (signed char) 127);
        EXPECT_EQ(packet->squelch_right.section.data, (signed char) 127);

        //set to -100 (invalid should set to 0)
        EXPECT_EQ(set_squelch_left(packet, -100) , 0);
        EXPECT_EQ(set_squelch_right(packet, -100), 0);
        EXPECT_EQ(packet->squelch_left.section.data, (signed char) 0);
        EXPECT_EQ(packet->squelch_right.section.data, (signed char) 0);

        //test we properly handle a null pointer
        free(packet);
        packet = NULL;
        ASSERT_NO_THROW(set_squelch_left(NULL, 666));
        ASSERT_NO_THROW(set_squelch_right(NULL, 666));
        ASSERT_EQ(set_squelch_left(NULL, 666), 1);
        ASSERT_EQ(set_squelch_right(NULL, 666), 1);
}

