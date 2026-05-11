# Hacking a USB-C Multimeter

Turning a cheap YOJOCK/Kowsi USB-C multimeter into a capable PY32 development board.

This repo is a write-up for the teardown, reverse engineering, and firmware for a low-cost USB-C multimeter based on a PY32F002A microcontroller. The goal is to repurpose the device as a compact development board with built-in USB-C power sensing circuitry, a display, ADC inputs, and exposed debug/programming access.

The firmware currently adds the following functionality:

- USB-C multimeter display
- Counter with persistent storage
- Flappy Bird game
- Stopwatch

<video src="https://github.com/user-attachments/assets/8cb6b859-e34d-4e74-b61d-6d0ab790db91" controls width="640"></video>

<img width="936" height="489" alt="USB-C multimeter" src="https://github.com/user-attachments/assets/daeccd54-a6c3-4796-a75b-6ac05817aae8" />

---

## Overview

This board is sold as a small USB-C power meter. It contains a PY32F002A microcontroller, an SPI TFT display, current/voltage sensing circuitry, and enough I/O to make it a good target for hacking.

The firmware is based on a modified version of [`IOsetting/py32f0-template`](https://github.com/IOsetting/py32f0-template), adapted specifically for this device.

---

## Hardware

### Main Components

| Component | Details |
|---|---|
| MCU | PY32F002A, TSOP-20 |
| CPU | 32-bit ARM Cortex-M |
| Flash | 20 KB |
| SRAM | 3 KB |
| Display | 160x180 SPI TFT, RGB565 |
| Interfaces | ADC, SPI, I2C, SWD |
| Clock | Built-in oscillator |

<img width="976" height="670" alt="PCB teardown" src="https://github.com/user-attachments/assets/5f5a09ec-1e07-486f-b81a-634f81eb69bb" />

---

## Firmware Setup

### 1. Install `pyOCD`

```bash
pip install pyocd
```

`pyOCD` is used to flash and debug the PY32 target over SWD.

I used this cheap DAPLINK for flashing: https://www.amazon.com/dp/B0CDWTVW7F

---

### 2. Clone the PY32F0 template

```bash
gh repo clone IOsetting/py32f0-template
```

This project uses a modified version of that template.

---

### 3. Install the ARM GNU Toolchain

Download the ARM GNU toolchain from Arm:

https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads

This project was tested with: `arm-gnu-toolchain-15.2.rel1-x86_64-arm-none-eabi` (12.2 also works)

Install it under `/opt/gcc-arm/`:

```bash
sudo mkdir -p /opt/gcc-arm/
sudo tar xvf arm-gnu-toolchain-15.2.rel1-x86_64-arm-none-eabi.tar.xz -C /opt/gcc-arm/

cd /opt/gcc-arm/
sudo chown -R root:root arm-gnu-toolchain-15.2.rel1-x86_64-arm-none-eabi/
```

---

### 4. Configure the Makefile

Update the Makefile for this target:

```make
MCU_TYPE        ?= PY32F002Ax5
FLASH_PROGRM   ?= pyocd
USE_FREERTOS   ?= n
ARM_TOOLCHAIN  ?= /opt/gcc-arm/arm-gnu-toolchain-15.2.rel1-x86_64-arm-none-eabi/bin
```

---

### 5. Modify the `User/` files

The `User/` folder contains the project specific code. Modify the `.c` and `.h` files in this folder, then compile and flash the firmware to the board.

Build and flash:

```bash
make clean
make
make flash
```

---

## Display

The board uses a 160x180 SPI TFT display with RGB565 color.

The ST7735S startup sequence was referenced from this implementation:

https://github.com/bersch/ST7735S/blob/master/st7735s.c

---

## Reverse-Engineered PCB

The PCB has been partially reverse engineered. Most components were identified and matched with parts from the JLCPCB/JLC Parts library.

Some items are still incomplete. The KiCad schematics are uploaded to this repo for anyone with more PCB reverse-engineering experience.

<img width="1510" height="851" alt="Reverse engineered PCB" src="https://github.com/user-attachments/assets/af3835c0-cb7f-40cc-8f42-8d0494ac2d02" />

---

## Notes

This is a work in progress or possibly abandoned by now.

ChatGPT was used for the .h/non-main.c files; AI was used to bit-bang a faster SPI interface, make sense of the complicated HAL interfacing, and research the reference manual for GPIO port addresses and offsets. The `main.c` file is mostly human-written by yours truly. I am neither a C or electrical engineering expert, this was made for learning and to make a hacker's dream come true. 

---

## Credits

- Original PY32F0 template: https://github.com/IOsetting/py32f0-template
- ST7735S reference implementation: https://github.com/bersch/ST7735S
