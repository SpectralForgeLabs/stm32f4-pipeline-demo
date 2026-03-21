cmake_minimum_required(VERSION 3.20)
project(stm32 C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Prebuild step
include(cmake/pre_build.cmake)

if(DEFINED ENV{GITHUB_RUN_NUMBER})
    set(BUILD_NUM $ENV{GITHUB_RUN_NUMBER})
else()
    set(BUILD_NUM 9999)
endif()

if(DEFINED ENV{GITHUB_SHA})
    string(SUBSTRING "$ENV{GITHUB_SHA}" 0 8 BUILD_SHA)
else()
    set(BUILD_SHA "local")
endif()

message(STATUS "Build number: ${BUILD_NUM}")

if(DEFINED ENV{GITHUB_ACTIONS})
# ---- FetchContent ----
  include(FetchContent)
  FetchContent_Declare(cmsis_core
      GIT_REPOSITORY https://github.com/ARM-software/CMSIS_6.git
      GIT_TAG        v6.3.0
      GIT_SHALLOW    TRUE
  )

  FetchContent_Declare(cmsis_f4
      GIT_REPOSITORY https://github.com/STMicroelectronics/cmsis-device-f4.git
      GIT_TAG        v2.6.11
      GIT_SHALLOW    TRUE
  )

  FetchContent_Declare(freertos_kernel
      GIT_REPOSITORY https://github.com/FreeRTOS/FreeRTOS-Kernel.git
      GIT_TAG        V11.2.0
      GIT_SHALLOW    TRUE
  )
  # Populate but don't process their CMakeLists
  FetchContent_Populate(freertos_kernel)
  FetchContent_Populate(cmsis_core)
  FetchContent_Populate(cmsis_f4)

  # ---- Paths now point to fetched content ----
  set(CMSIS_CORE_DIR  ${cmsis_core_SOURCE_DIR})
  set(ST_F4_DIR       ${cmsis_f4_SOURCE_DIR})
  set(FREERTOS_DIR    ${freertos_kernel_SOURCE_DIR})
else()
  # ---- Paths to your shared "layers" ----
  set(CMSIS_CORE_DIR "$ENV{HOME}/.embedded/cmsis_core")
  set(ST_F4_DIR      "$ENV{HOME}/.embedded/st/cmsis-device-f4")
  set(FREERTOS_DIR   "$ENV{HOME}/.embedded/FreeRTOS-Kernel")
endif()
  
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
include(bsp/bsp.cmake)
include(drivers/drivers.cmake)

# ---- Target ----
add_executable(${PROJECT_NAME}.elf
  src/main.c
  src/syscall.c
  src/spectral_framer.c
  src/spectral_parser.c
  src/clock_stm32f4xx.c
  src/tasks/blinky_task.c
  src/tasks/uart_task.c
  ${ST_F4_DIR}/Source/Templates/system_stm32f4xx.c
  ${ST_F4_DIR}/Source/Templates/gcc/startup_stm32f407xx.s
)

target_include_directories(${PROJECT_NAME}.elf PRIVATE
  ${CMSIS_CORE_DIR}/CMSIS/Core/Include
  ${ST_F4_DIR}/Include
  ${CMAKE_SOURCE_DIR}/inc
)

# Required so stm32f4xx.h pulls the right part header
target_compile_definitions(${PROJECT_NAME}.elf PRIVATE
  STM32F407xx
  BUILD_NUM=${BUILD_NUM}
  BUILD_SHA="${BUILD_SHA}"
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

target_link_libraries(${PROJECT_NAME}.elf PRIVATE 
  freertos
  bsp
  drivers
  )

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
# Compute short SHA from BUILD_NUMBER is already set, grab SHA too
string(SUBSTRING "${GIT_SHA}" 0 7 SHORT_SHA)

# ---- Generate .hex and .bin ----
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
  

  COMMAND ${CMAKE_OBJCOPY} -O ihex
          $<TARGET_FILE:${PROJECT_NAME}.elf>
          ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-${BUILD_NUM}-${SHORT_SHA}.hex
  COMMAND ${CMAKE_OBJCOPY} -O binary
          $<TARGET_FILE:${PROJECT_NAME}.elf>
          ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-${BUILD_NUM}-${SHORT_SHA}.bin

# Rename ELF with build number
  COMMAND ${CMAKE_COMMAND} -E rename
          $<TARGET_FILE:${PROJECT_NAME}.elf>
          ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-${BUILD_NUM}-${SHORT_SHA}.elf
  BYPRODUCTS
          ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-${BUILD_NUM}-${SHORT_SHA}.hex
          ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-${BUILD_NUM}-${SHORT_SHA}.bin

  COMMENT "Packaging: ${PROJECT_NAME}-${BUILD_NUMBER}-${SHORT_SHA}.{elf,hex,bin}"
)
