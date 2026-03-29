cmake_minimum_required(VERSION 3.20)

add_library(bsp_uart STATIC
    ${CMAKE_CURRENT_LIST_DIR}/uart.c
)

target_link_libraries(bsp_uart 
    PRIVATE
    freertos
)

target_include_directories(bsp_uart
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMSIS_CORE_DIR}/CMSIS/Core/Include
    ${ST_F4_DIR}/Include
    ${CMAKE_SOURCE_DIR}/inc
    # ${FREERTOS_DIR}/include
)

target_compile_definitions(bsp_uart
    PUBLIC
    STM32F407xx
)

target_compile_options(bsp_uart
    PRIVATE
    ${MCU_FLAGS}
    ${COMMON_C_FLAGS}
    -O0
    -g3
)