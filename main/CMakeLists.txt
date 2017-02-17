cmake_minimum_required(VERSION 3.5)

set(Main_SRCS
    main.c
)

add_executable(rt8900c
    ${Main_SRCS}
)

# Specify here the libraries this program depends on
target_link_libraries(rt8900c
    librt8900
)

install(TARGETS rt8900c DESTINATION bin)
