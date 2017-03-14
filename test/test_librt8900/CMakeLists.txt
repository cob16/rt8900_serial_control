file(GLOB SRCS *.cpp)

include_directories(${GTEST_INCLUDE_DIRS} ${COMMON_INCLUDES})

ADD_EXECUTABLE(test_librt8900 ${SRCS})

TARGET_LINK_LIBRARIES(test_librt8900
                      librt8900
                      libgtest
)

add_test(NAME test_librt8900
         COMMAND test_librt8900)