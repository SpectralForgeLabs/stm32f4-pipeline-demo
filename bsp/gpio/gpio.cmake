cmake_minimum_required(VERSION 3.20)

add_library(bsp_gpio STATIC
    ${CMAKE_CURRENT_LIST_DIR}/gpio.c
)

target_include_directories(bsp_gpio
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMSIS_CORE_DIR}/CMSIS/Core/Include
    ${ST_F4_DIR}/Include
)

target_compile_definitions(bsp_gpio
    PUBLIC
    STM32F407xx
)

target_compile_options(bsp_gpio
    PRIVATE
    ${MCU_FLAGS}
    ${COMMON_C_FLAGS}
    -O0
    -g3
)