project(${CMAKE_PROJECT_NAME} C)

set(librt8900_SRCS
        serial.c serial.h
        control_packet.h control_packet.c
        packet.h packet.c
        log.h log.c
        display_packet.c display_packet.h
        librt8900.c librt8900.h)

# Declare the library
add_library(librt8900 STATIC
    ${librt8900_SRCS}
)

# Specify here the libraries this program depends on
#target_link_libraries(librt8900)

# Specify here the include directories exported
# by this library
target_include_directories(librt8900 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
