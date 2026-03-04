cmake_minimum_required(VERSION 3.20)
project(stm_test C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ---- Paths to your shared "layers" ----
set(CMSIS_CORE_DIR "$ENV{HOME}/.embedded/cmsis_core")
set(ST_F4_DIR      "$ENV{HOME}/.embedded/st/cmsis-device-f4")

set(LINKER_SCRIPT  "${CMAKE_SOURCE_DIR}/linker/stm32f407vet6.ld")

# ---- MCU flags (STM32F407 = Cortex-M4F, hard-float) ----
set(MCU_FLAGS 
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
    )

# Common warnings/size flags (keep it sane)
set(COMMON_C_FLAGS
    -ffunction-sections
    -fdata-sections
    -fno-common
    -Wall
    -Wextra
    )
set(COMMON_LD_FLAGS
    -Wl,--gc-sections
    -Wl,-Map=${CMAKE_BINARY_DIR}/${PROJECT_NAME}.map
    )

# Add other cmake directories here
include(cmake/freertos.cmake)

# ---- Target ----
add_executable(${PROJECT_NAME}.elf
  src/main.c
  src/syscall.c
  src/clock_stm32f4xx.c
  src/drivers/uart.c
  src/drivers/spi.c
  src/drivers/timer.c
  src/tasks/blinky_task.c
  src/tasks/uart_task.c
  ${ST_F4_DIR}/Source/Templates/system_stm32f4xx.c
  ${ST_F4_DIR}/Source/Templates/gcc/startup_stm32f407xx.s
)

target_include_directories(${PROJECT_NAME}.elf PRIVATE
  ${CMSIS_CORE_DIR}/CMSIS/Core/Include
  ${ST_F4_DIR}/Include
  ${CMAKE_SOURCE_DIR}/include
)

# Required so stm32f4xx.h pulls the right part header
target_compile_definitions(${PROJECT_NAME}.elf PRIVATE
  STM32F407xx
)

target_compile_options(${PROJECT_NAME}.elf PRIVATE
  ${MCU_FLAGS}
  ${COMMON_C_FLAGS}
  -O0
  -g3
)

# Let assembler see the same CPU flags
set_source_files_properties(
  ${ST_F4_DIR}/Source/Templates/gcc/startup_stm32f407xx.s
  PROPERTIES COMPILE_OPTIONS "${MCU_FLAGS}"
)

target_link_libraries(${PROJECT_NAME}.elf PRIVATE freertos)

target_link_options(${PROJECT_NAME}.elf PRIVATE
  ${MCU_FLAGS}
  -T${LINKER_SCRIPT}
  ${COMMON_LD_FLAGS}
  -Wl,--print-memory-usage
  --specs=nosys.specs
  --specs=nano.specs
)

# Optional: print size after link
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
  COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}.elf>
)

# ---- Generate .hex and .bin ----
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O ihex
          $<TARGET_FILE:${PROJECT_NAME}.elf>
          ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.hex
  COMMAND ${CMAKE_OBJCOPY} -O binary
          $<TARGET_FILE:${PROJECT_NAME}.elf>
          ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin
  BYPRODUCTS
          ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.hex
          ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin
)
