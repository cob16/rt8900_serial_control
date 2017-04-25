//
// Created by cormac on 25/04/17.
//

#ifndef RT8900_SERIAL_CONTROL_TEST_LIBRT8900_H
#define RT8900_SERIAL_CONTROL_TEST_LIBRT8900_H

#include "gtest/gtest.h"
extern "C" {
#include "librt8900.h"
}

// The fixture for testing class Foo.
class Librt8900Test : public ::testing::Test {

protected:

    Librt8900Test();

//    virtual ~ControlPacketTest();
//    virtual void SetUp();
//    virtual void TearDown();
};

#endif //RT8900_SERIAL_CONTROL_TEST_LIBRT8900_H
