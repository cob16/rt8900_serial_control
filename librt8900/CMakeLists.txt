set(librt8900_SRCS
    control_packet.h
    control_packet.c
        serial.c packet.c packet.h)

# Declare the library
add_library(librt8900 STATIC
    ${librt8900_SRCS}
)

# Specify here the include directories exported
# by this library
target_include_directories(librt8900 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
