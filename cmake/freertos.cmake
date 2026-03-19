# cmake/freertos.cmake
set(FREERTOS_DIR ${FREERTOS_DIR})

add_library(freertos STATIC
    # Kernel
    ${FREERTOS_DIR}/list.c
    ${FREERTOS_DIR}/queue.c
    ${FREERTOS_DIR}/tasks.c
    ${FREERTOS_DIR}/timers.c
    ${FREERTOS_DIR}/event_groups.c
    ${FREERTOS_DIR}/stream_buffer.c
    ${FREERTOS_DIR}/croutine.c

    # Port (Cortex-M4F GCC)
    ${FREERTOS_DIR}/portable/GCC/ARM_CM4F/port.c

    # Heap
    ${FREERTOS_DIR}/portable/MemMang/heap_4.c
)

target_include_directories(freertos PUBLIC
    ${FREERTOS_DIR}/include
    ${FREERTOS_DIR}/portable/GCC/ARM_CM4F

    # Wherever FreeRTOSConfig.h lives:
    ${CMAKE_SOURCE_DIR}/inc
)

# Optional but nice: enable some warnings just for RTOS, or keep it minimal:
target_compile_options(freertos PRIVATE
    ${MCU_FLAGS}
    ${COMMON_C_FLAGS}
    -O0 -g3
)