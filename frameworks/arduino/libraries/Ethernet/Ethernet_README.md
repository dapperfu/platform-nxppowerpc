# Ethernet Library for MPC5744P

## Important Note

**The DEVKIT-MPC5744P uses the 144 LQFP package which does NOT include an Ethernet (ENET) peripheral.**

According to the [DEVKIT-MPC5744P Quick Start Guide](https://www.nxp.com/docs/en/quick-reference-guide/MPC5744P-DEV-KIT-REVB-QSG.pdf):

- **144 LQFP package**: Does NOT have ENET
- **257 MAPBGA package**: Has ENET support

The Ethernet library is provided for compatibility with Arduino-style APIs, but **will not function on the standard DEVKIT-MPC5744P board** (144 LQFP).

If you need Ethernet support, you would need:
1. A custom board with the 257 MAPBGA package variant
2. Or use external Ethernet controller via SPI or other interface

## Package Comparison

| Package | ENET Support | Available on DEVKIT-MPC5744P |
|---------|-------------|------------------------------|
| 144 LQFP | No | Yes (standard) |
| 257 MAPBGA | Yes | No |

## Usage

If you have a 257 MAPBGA variant, you can use the Ethernet library. Otherwise, the library functions will not have hardware support.

For DEVKIT-MPC5744P, consider using:
- CAN communication (CAN1 on PA14/PA15)
- UART/Serial communication
- SPI communication
- Other available peripherals

