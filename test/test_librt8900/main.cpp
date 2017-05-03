#include "gtest/gtest.h"

extern "C" {
#include "librt8900.h"
}

class Rt8900Environment : public testing::Environment{
public:
    virtual void SetUp()
    {
        /* Lower the logging level so as to not show warnings.
        * For this test a warning is the expected behaviour,
        * However we test differently and do not what in the stdout */
            set_log_level(RT8900_ERROR);
    }
};

int main(int argc, char **argv)
{
        ::testing::InitGoogleTest(&argc, argv);
        ::testing::AddGlobalTestEnvironment(new Rt8900Environment);
        int ret = RUN_ALL_TESTS();
        return ret;
}
