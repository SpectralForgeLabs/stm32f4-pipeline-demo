# STM32F407 FreeRTOS CI Pipeline Demo ![Build](https://github.com/SpectralForgeLabs/stm32f4-pipeline-demo/actions/workflows/k3s-build.yml/badge.svg)

A bare-metal embedded firmware project for the STM32F407VET6 (Cortex-M4F) 
demonstrating a complete, reproducible CI/CD build pipeline — no STM32CubeIDE, 
no HAL bloat, no "works on my machine."

---

## What this is
This repository is both a reference implementation and a deployable starting point.

This project demonstrates what a **production-grade embedded CI/CD pipeline** 
looks like for small teams.

If your current process is:
> "Build it on Dave's machine"

This is the alternative:
- Reproducible builds
- Automated testing
- Traceable artifacts
- Optional hardware-in-the-loop (HIL) validation

---

## Hardware

- STM32F407VET6 (Cortex-M4F, running on internal 16MHz HSI clock)
- FreeRTOS V11.2.0
- Bare register-level drivers (no STM32Cube HAL)
- Active peripherals:
  - UART (DMA)
  - SPI (DMA)
  - Timer (us)
  - RTOS task scheduling (blinky)

---

## Architecture Overview

```text
Developer Push (GitHub)
        |
        v
GitHub Actions Workflow
        |
        v
Runner (Raspberry Pi / k3s cluster)
        |
        v
Dockerized Build (CMake + Ninja + ARM GCC)
        |
        v
Artifacts Generated (.elf / .hex / .bin)
        |
        +------------------------+------------------------+
        |                                                |
        v                                                v
GitHub Artifacts                                 Nexus Repository
(Simple Pipeline)                               (Scalable Pipeline)
        |                                                |
        +------------------------+------------------------+
                                 |
                                 v
                   Hardware-in-the-Loop (HIL)
            (Download Artifact -> Flash -> UART Validate)
                                 |
                                 v
                          Pass / Fail Result
```

---

## Pipeline Overview

This repository demonstrates **two CI/CD approaches**, depending on team size and infrastructure:

### 1. Single Runner Pipeline (Simple Setup)

- Runs on a self-hosted Raspberry Pi
- Uses GitHub Actions only
- Builds firmware in Docker
- Uploads artifacts to GitHub

**Use case:**
- Small teams
- No infrastructure
- Quick setup

---

### 2. Scalable Pipeline (k3s + Nexus)

- Runs on a Kubernetes (k3s) cluster
- Distributed self-hosted runners
- Dockerized cross-compilation
- Artifacts stored in Nexus repository tagged with SHA + build number
- Supports hardware-in-the-loop (HIL) testing

**Use case:**
- Growing teams
- Multiple developers
- Need for traceability, scaling, and artifact management

---

## Workflow Behavior

- **Push to feature branches**
  - Triggers scalable pipeline (k3s)
  - Runs build + test

- **Pull Requests → master**
  - Full validation pipeline runs

- **Push to master**
  - Triggers single-runner pipeline
  - Produces downloadable artifacts via GitHub

This separation demonstrates how teams can evolve from simple CI 
to scalable infrastructure without rewriting everything.

---

## Build System

- ARM GCC Toolchain (arm-none-eabi 15.2)
- CMake + Ninja
- Fully containerized builds (Docker)
- No IDE dependency

---

## Artifacts

Two artifact strategies are demonstrated:

### GitHub Artifacts (Simple Pipeline)
- Available directly in GitHub Actions
- Easy to access
- Short-term storage

### Nexus Repository (Scalable Pipeline)
- Centralized artifact storage
- Versioned firmware builds
- Suitable for team-wide usage and traceability

Artifacts include:
- `.elf`
- `.hex`
- `.bin`

---

## Dependencies

Fetched automatically at build time — no manual setup:

| Library | Version |
|--------|--------|
| ARM CMSIS Core | v6.3.0 |
| STM32F4 CMSIS Device | v2.6.11 |
| FreeRTOS Kernel | V11.2.0 |

- CI builds use FetchContent
- Local builds use `$HOME/.embedded/`

---

## Building Locally

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-gcc.cmake
cmake --build build
```

## About

Built by an embedded software engineer with production VxWorks experience.

Available for consulting on:

- Embedded CI/CD pipelines
- Hardware-in-the-loop (HIL) automation
- Cross-compilation environments
- Self-hosted runners (Local machines / k3s)
- Artifact management (Nexus)

Contact: [LinkedIn](https://www.linkedin.com/in/daniel-guillen-b7b239132/) 
