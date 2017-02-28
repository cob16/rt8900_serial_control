set(io_queue_SRCS
        io_queue.h
)

# Declare the library
add_library(io_queue STATIC
        ${io_queue_SRCS}
)

SET_TARGET_PROPERTIES(io_queue PROPERTIES LINKER_LANGUAGE C)

# Specify here the include directories exported
# by this library
target_include_directories(io_queue PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
