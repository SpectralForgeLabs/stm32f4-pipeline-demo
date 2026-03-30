cmake_minimum_required(VERSION 3.20)
project(spectral_tests C)

enable_testing()

set(CMAKE_C_STANDARD 11)

# Unity from baked image
set(UNITY_DIR $ENV{UNITY_DIR})

add_library(unity STATIC
    ${UNITY_DIR}/src/unity.c
)
target_include_directories(unity PUBLIC
    ${UNITY_DIR}/src
)

# Shared source under test
set(SRC_ROOT ${CMAKE_SOURCE_DIR}/..)

# Stubs
add_library(stubs STATIC
    stubs/uart_stub.c
    stubs/buildinfo_stub.c
    stubs/gpio_stub.c
    stubs/os_write_stub.c
)
target_include_directories(stubs PUBLIC
    stubs/
    ${SRC_ROOT}/inc
)

include(framer/framer.cmake)
include(parser/parser.cmake)