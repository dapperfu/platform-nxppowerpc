# NXP PowerPC VLE: development platform for [PlatformIO](https://platformio.org)

NXP PowerPC VLE development platform for PlatformIO, supporting MPC-5748G and MPC-5748P microcontrollers using Docker-based cross-compilation.

## Overview

This platform provides support for NXP PowerPC VLE (Variable Length Encoding) microcontrollers, specifically the MPC57xx series. It uses a Docker-based toolchain from [AutomotiveDevOps/powerpc-eabivle-gcc-dockerfiles](https://github.com/AutomotiveDevOps/powerpc-eabivle-gcc-dockerfiles) for consistent, reproducible builds across different development environments.

## Supported Boards

- **NXP MPC-5748G**: 32-bit PowerPC e200z4 MCU, 120 MHz
- **NXP MPC-5748P**: 32-bit PowerPC e200z4 MCU, 120 MHz

## Prerequisites

### Docker Setup

1. Install Docker on your system:
   ```bash
   # Ubuntu/Debian
   sudo apt-get install docker.io
   
   # Or follow Docker's official installation guide
   ```

2. Build the Docker image:
   ```bash
   git clone https://github.com/AutomaticDevOps/powerpc-eabivle-gcc-dockerfiles.git
   cd powerpc-eabivle-gcc-dockerfiles
   sh build.sh
   ```

   This will create a Docker image tagged as `s32ds-power-v1-2:latest`.

3. Verify the image exists:
   ```bash
   docker images | grep s32ds-power-v1-2
   ```

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
platform = https://github.com/platformio/platform-nxppowerpc.git
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

## Docker Toolchain

The platform uses Docker container `s32ds-power-v1-2:latest` which includes:
- `powerpc-eabivle-gcc` - C compiler
- `powerpc-eabivle-g++` - C++ compiler
- `powerpc-eabivle-ar` - Archiver
- `powerpc-eabivle-objcopy` - Object file converter
- `powerpc-eabivle-size` - Size utility
- Other standard GCC toolchain utilities

All compilation happens inside the Docker container, ensuring consistent builds regardless of the host system.

## Troubleshooting

### Docker Image Not Found

If you see errors about the Docker image:
```bash
# Check if image exists
docker images | grep s32ds-power-v1-2

# If missing, build it:
cd powerpc-eabivle-gcc-dockerfiles
sh build.sh
```

### Docker Permission Issues

On Linux, you may need to add your user to the docker group:
```bash
sudo usermod -aG docker $USER
# Log out and back in
```

### Linker Errors

Ensure you have:
1. A valid linker script specified in `build_flags`
2. Startup code in `src/` directory
3. Entry point defined (typically `main` or custom)

## Examples

See `examples/baremetal-blink/` for a simple example project.

## Contributing

Contributions welcome! Areas for improvement:
- FreeRTOS framework support
- Arduino framework support
- Additional board support
- Upload/debug support (OpenSDA, etc.)
- Better Docker integration

## License

Apache License 2.0

## Resources

- [NXP MPC5748G Product Page](https://www.nxp.com/products/processors-and-microcontrollers/power-architecture-processors/powerpc-processors/powerpc-5xx-processors/mpc5748g-32-bit-microcontroller-mcu:MPC5748G)
- [PowerPC e200z4 Core Reference](https://www.nxp.com/docs/en/reference-manual/MPC57XXRM.pdf)
- [Docker Toolchain Repository](https://github.com/AutomotiveDevOps/powerpc-eabivle-gcc-dockerfiles)
- [PlatformIO Documentation](https://docs.platformio.org/)

