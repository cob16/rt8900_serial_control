//
// Created by cormac on 14/03/17.
//

#ifndef RT8900_SERIAL_CONTROL_TEST_DISPLAY_PACKET_H
#define RT8900_SERIAL_CONTROL_TEST_DISPLAY_PACKET_H

#include "gtest/gtest.h"
#include "librt8900.h"

class TestDisplayPacket : public ::testing::Test {};

class TestDisplayPacketReaders : public ::testing::Test {
public:
    virtual void SetUp();

    struct display_packet packet;
};

#endif //RT8900_SERIAL_CONTROL_TEST_DISPLAY_PACKET_H
