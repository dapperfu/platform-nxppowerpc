# NXP PowerPC VLE: development platform for [PlatformIO](https://platformio.org)

NXP PowerPC VLE development platform for PlatformIO, supporting MPC-5748G and MPC-5748P microcontrollers using PlatformIO's standard toolchain package system.

## Overview

This platform provides support for NXP PowerPC VLE (Variable Length Encoding) microcontrollers, specifically the MPC57xx series. It uses PlatformIO's toolchain package system which automatically downloads and manages the PowerPC EABI VLE cross-compilation toolchain.

## Supported Boards

- **NXP MPC-5748G**: 32-bit PowerPC e200z4 MCU, 120 MHz
- **NXP MPC-5748P**: 32-bit PowerPC e200z4 MCU, 120 MHz

## Prerequisites

No manual setup required! PlatformIO will automatically download and install the required PowerPC EABI VLE toolchain package when you install this platform.

## Installation

### Stable Version

Add to your `platformio.ini`:

```ini
[env:mpc5748g]
platform = nxppowerpc
board = mpc5748g
framework = baremetal
```

### Development Version

```ini
[env:mpc5748g]
platform = https://github.com/dapperfu/platform-nxppowerpc.git
board = mpc5748g
framework = baremetal
```

Or install locally:

```bash
pio platform install /path/to/platform-nxppowerpc
```

## Usage

### Basic Project Setup

1. Create a new project:
   ```bash
   pio project init --board mpc5748g
   ```

2. Configure `platformio.ini`:
   ```ini
   [env:mpc5748g]
   platform = nxppowerpc
   board = mpc5748g
   framework = baremetal
   
   ; Build flags
   build_flags =
       -Os
       -Wall
       -Wextra
       ; Linker script (provide your own)
       ; -T path/to/linker.ld
   ```

3. Add source files to `src/` directory:
   ```c
   // src/main.c
   int main(void) {
       // Your code here
       while(1) {
           // Main loop
       }
       return 0;
   }
   ```

4. Build:
   ```bash
   pio run
   ```

### Linker Script

For baremetal development, you need to provide a linker script. Common options:

1. Place `linker.ld` in your project root and specify:
   ```ini
   build_flags = -T linker.ld
   ```

2. Or use a path relative to your project:
   ```ini
   build_flags = -T ${PROJECT_DIR}/linker.ld
   ```

### Startup Code

Provide your startup code (e.g., `startup.S` or `startup.c`) in the `src/` directory. PlatformIO will automatically compile it.

### Build Output

The build process produces:
- `firmware.elf` - ELF executable
- `firmware.bin` - Binary file (via `pio run -t bin`)

### Size Information

View binary size:
```bash
pio run -t size
```

## Frameworks

### Baremetal

Minimal framework with no runtime. You provide:
- Startup code
- Linker script
- All application code

Future frameworks (not yet implemented):
- **FreeRTOS**: Real-time operating system support
- **Arduino**: Arduino API compatibility

## Build Flags

Common build flags for PowerPC VLE:

```ini
build_flags =
    ; Optimization
    -Os                          ; Optimize for size
    ; -O2                        ; Optimize for performance
    
    ; Warnings
    -Wall                        ; All warnings
    -Wextra                      ; Extra warnings
    
    ; Linker script
    -T ${PROJECT_DIR}/linker.ld  ; Custom linker script
    
    ; Preprocessor defines
    -DF_CPU=120000000            ; CPU frequency
    -DMPC5748G                   ; Board define (auto-set)
```

## Toolchain

The platform automatically installs the `toolchain-powerpc-eabivle` package which includes:
- `powerpc-eabivle-gcc` - C compiler
- `powerpc-eabivle-g++` - C++ compiler
- `powerpc-eabivle-ar` - Archiver
- `powerpc-eabivle-objcopy` - Object file converter
- `powerpc-eabivle-size` - Size utility
- Other standard GCC toolchain utilities

PlatformIO automatically manages the toolchain package, downloading and extracting it to `.platformio/packages/toolchain-powerpc-eabivle/` in your project directory. This ensures consistent builds across different development environments without requiring Docker or manual toolchain installation.

## Troubleshooting

### Toolchain Not Found

If you see errors about the toolchain:
```bash
# Reinstall the platform to force toolchain download
pio platform uninstall nxppowerpc
pio platform install nxppowerpc

# Or install from git
pio platform install https://github.com/dapperfu/platform-nxppowerpc.git
```

### Linker Errors

Ensure you have:
1. A valid linker script specified in `build_flags`
2. Startup code in `src/` directory
3. Entry point defined (typically `main` or custom)

## Examples

Examples are maintained in a separate repository to keep the platform codebase clean:

**[platform-nxppowerpc-examples](https://github.com/dapperfu/platform-nxppowerpc-examples)**

Available examples include:
- **FreeRTOS Blink**: LED blink using FreeRTOS tasks
- **Baremetal Blink**: Minimal baremetal example

Clone the examples repository:
```bash
git clone https://github.com/dapperfu/platform-nxppowerpc-examples.git
cd platform-nxppowerpc-examples/freertos-blink
pio run
```

## Contributing

Contributions welcome! Areas for improvement:
- FreeRTOS framework support
- Arduino framework support
- Additional board support
- Upload/debug support (OpenSDA, etc.)

## License

Apache License 2.0

## Resources

- [NXP MPC5748G Product Page](https://www.nxp.com/products/processors-and-microcontrollers/power-architecture-processors/powerpc-processors/powerpc-5xx-processors/mpc5748g-32-bit-microcontroller-mcu:MPC5748G)
- [PowerPC e200z4 Core Reference](https://www.nxp.com/docs/en/reference-manual/MPC57XXRM.pdf)
- [PlatformIO Documentation](https://docs.platformio.org/)

