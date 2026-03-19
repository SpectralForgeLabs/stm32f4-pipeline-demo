# STM32F407 FreeRTOS CI Pipeline Demo

A bare-metal embedded firmware project for the STM32F407VET6 (Cortex-M4F) 
demonstrating a complete, reproducible CI/CD build pipeline — no STM32CubeIDE, 
no HAL bloat, no "works on my machine."

## What this is

This project exists to show small embedded teams what a professional build 
pipeline looks like. If your current process is "build it on Dave's laptop 
and hope for the best," this is the alternative.

## Hardware

- STM32F407VET6 (Cortex-M4F, 168MHz, hard-float)
- FreeRTOS V11.2.0
- Bare register-level drivers (no STM32Cube HAL)
- Active peripherals: UART DMA, SPI DMA, blinky task

## Build pipeline

Every commit triggers an automated build via GitHub Actions on a 
self-hosted Raspberry Pi runner:

- ARM toolchain in Docker (arm-none-eabi-gcc 13.x)
- CMake + Ninja build system
- Produces ELF, HEX, and BIN artifacts automatically
- Build artifacts available for download on every commit

![Build](https://github.com/YOUR_USERNAME/stm32-freertos-ci-pipeline/actions/workflows/build.yml/badge.svg)

## Dependencies

Fetched automatically at build time — no manual setup:

| Library | Version |
|---|---|
| ARM CMSIS Core | v6.3.0 |
| STM32F4 CMSIS Device | v2.6.11 |
| FreeRTOS Kernel | V11.2.0 |

## Building locally
```bash
cmake -B build -G Ninja \
  -DCMSIS_CORE_DIR=/path/to/cmsis_core \
  -DST_F4_DIR=/path/to/cmsis-device-f4
cmake --build build
```

## About

Built by an embedded software engineer with production VxWorks experience. 
Available for consulting on embedded CI/CD pipeline setup for small teams.
Contact: [your email or LinkedIn]

<!-- 
UART to USB
 * Green TX
 * White RX
 * Black GND

Repos needed
branch v6.3.0 git@github.com:ARM-software/CMSIS_6.git cmsis_core
branch v2.6.11 git@github.com:STMicroelectronics/cmsis-device-f4.git
branch V11.2.0 git@github.com:FreeRTOS/FreeRTOS-Kernel.git
-->
