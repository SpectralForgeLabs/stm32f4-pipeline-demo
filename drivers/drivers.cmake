cmake_minimum_required(VERSION 3.20)

include(${CMAKE_CURRENT_LIST_DIR}/spi_flash/spi-flash.cmake)

add_library(drivers INTERFACE)

target_link_libraries(drivers
    INTERFACE
    spi_flash
)