cmake_minimum_required(VERSION 3.5)
project(rt8900_serial_control)

set(CMAKE_C_STANDARD 11)

enable_testing()

add_subdirectory(main)
add_subdirectory(test)
add_subdirectory(librt8900)
add_subdirectory(include)
