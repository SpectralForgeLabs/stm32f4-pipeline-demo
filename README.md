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

- ARM toolchain in Docker (arm-none-eabi-gcc 15.2)
- CMake + Ninja build system
- Produces ELF, HEX, and BIN artifacts automatically
- Build artifacts available for download on every commit

![Build](https://github.com/SpectralForgeLabs/stm32f4-pipeline-demo/actions/workflows/k3s-build.yml/badge.svg)

## Dependencies

Fetched automatically at build time — no manual setup:

| Library | Version |
|---|---|
| ARM CMSIS Core | v6.3.0 |
| STM32F4 CMSIS Device | v2.6.11 |
| FreeRTOS Kernel | V11.2.0 |

## Building locally
```bash
cmake -S . -B build -G Ninja \
-DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-gcc.cmake
cmake --build build
```
Dependencies fetched automatically via FetchContent when building in CI.
Local builds use dependencies from `$HOME/.embedded/`.

## About

Built by an embedded software engineer with production VxWorks experience. 
Available for consulting on embedded CI/CD pipeline setup for small teams.

Contact: [LinkedIn](https://www.linkedin.com/in/daniel-guillen-b7b239132/) 
