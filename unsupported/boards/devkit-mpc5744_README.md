# DEVKIT-MPC5744P Board Configuration

## Board Overview

The DEVKIT-MPC5744P is an ultra-low-cost development platform for MPC5744P microcontrollers with Arduino UNO R3 footprint compatibility.

Reference: [DEVKIT-MPC5744P Quick Start Guide](https://www.nxp.com/docs/en/quick-reference-guide/MPC5744P-DEV-KIT-REVB-QSG.pdf)

## Hardware Specifications

- **MCU**: MPC5744P (144 LQFP package)
- **CPU**: 2 x 200 MHz Power Architecture e200Z4 Dual issue cores (delayed lockstep)
- **System Clock**: 160 MHz (PLL configured)
- **Flash**: 2.5 MB
- **RAM**: 384 KB
- **Package**: 144 LQFP (79 GPIO pins)

## CAN Configuration

The board provides **CAN1** on the following pins:

- **CAN1_TXD**: PA14 (J2-18) - Transmit
- **CAN1_RXD**: PA15 (J2-20) - Receive

The Arduino CAN library is configured to use CAN1 with:
- Default baudrate: 500 kbps
- Clock source: 40 MHz oscillator
- 64 message buffers (MPC5744P feature)

**Note**: The board has 3 FlexCAN modules available (CAN0, CAN1, CAN2), but only CAN1 is connected to accessible header pins on the standard DEVKIT-MPC5744P.

## Ethernet Configuration

**IMPORTANT**: The DEVKIT-MPC5744P uses the **144 LQFP package which does NOT include an Ethernet (ENET) peripheral**.

According to the MPC574xP Family specifications:
- **144 LQFP package**: No ENET support
- **257 MAPBGA package**: Has ENET support

The Ethernet library is provided for API compatibility but will not have hardware support on the standard DEVKIT-MPC5744P board.

## Available Peripherals (144 LQFP Package)

According to the board documentation:

- **FlexCAN**: 3 modules (CAN0, CAN1, CAN2)
- **LINFlexD**: 2 modules (LIN/UART)
- **SPI**: Available
- **FlexPWM**: 2 modules (limited channels on FlexPWM1)
- **eTimer**: 3 modules (eTimer2 has limited external signals)
- **ADC**: 4 x 12-bit ADCs with 16 channels each
- **eDMA**: 32 channels
- **CTU**: 2 modules
- **GPIO**: 79 pins (via SIUL2)

## Pin Mapping

The board follows Arduino UNO R3 footprint for shield compatibility. Pin mappings are defined in the Arduino core library (`wiring_digital.c`).

## System Configuration

- **CPU Frequency**: 160 MHz (160000000L)
- **Linker Script**: `57xx_flash.ld` (auto-detected)
- **Debug Interface**: OpenSDA (on-board)
- **Power Supply**: USB (micro-B) or external 12V

## Supported Frameworks

- baremetal
- freertos
- arduino

## References

- [DEVKIT-MPC5744P Quick Start Guide](https://www.nxp.com/docs/en/quick-reference-guide/MPC5744P-DEV-KIT-REVB-QSG.pdf)
- [MPC5744P Product Page](https://www.nxp.com/products/processors-and-microcontrollers/power-architecture-processors/powerpc-processors/powerpc-5xx-processors/mpc5744p-32-bit-microcontroller-mcu:MPC5744P)
- [NXP DEVKIT-MPC5744P Webpage](https://www.nxp.com/devkit-mpc5744p)

