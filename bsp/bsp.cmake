cmake_minimum_required(VERSION 3.20)

include(${CMAKE_CURRENT_LIST_DIR}/uart/uart.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/gpio/gpio.cmake)
# include(${CMAKE_CURRENT_LIST_DIR}/spi/spi.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/timer/timer.cmake)

add_library(bsp INTERFACE)

target_link_libraries(bsp
    INTERFACE
    bsp_uart
    bsp_gpio
    # bsp_spi
    bsp_timer
)