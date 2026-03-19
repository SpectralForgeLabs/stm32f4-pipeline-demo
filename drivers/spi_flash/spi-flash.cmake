cmake_minimum_required(VERSION 3.20)

add_library(spi_flash STATIC
    ${CMAKE_CURRENT_LIST_DIR}/spi_flash.c
)

target_include_directories(spi_flash
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMSIS_CORE_DIR}/CMSIS/Core/Include
    ${ST_F4_DIR}/Include
)

target_compile_definitions(spi_flash
    PUBLIC
    STM32F407xx
)

target_link_libraries(spi_flash PRIVATE
    bsp_uart 
    bsp_gpio
)


target_compile_options(spi_flash
    PRIVATE
    ${MCU_FLAGS}
    ${COMMON_C_FLAGS}
    -O0
    -g3
)