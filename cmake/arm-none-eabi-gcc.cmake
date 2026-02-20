set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Toolchain root
set(TOOLCHAIN_ROOT /opt/arm-none-eabi)

# Compilers
set(CMAKE_C_COMPILER   ${TOOLCHAIN_ROOT}/bin/arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_ROOT}/bin/arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_ROOT}/bin/arm-none-eabi-g++)

# Utilities
set(CMAKE_OBJCOPY ${TOOLCHAIN_ROOT}/bin/arm-none-eabi-objcopy)
set(CMAKE_SIZE    ${TOOLCHAIN_ROOT}/bin/arm-none-eabi-size)

# Avoid try-run tests
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
