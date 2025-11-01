# NXP PowerPC VLE: development platform for [PlatformIO](https://platformio.org)

## The Saga of the PowerPC IP Chain of Custody 🏛️

Once upon a time, in the hallowed halls of ~~Motorola~~ ~~Freescale~~ **NXP**, there lived some truly excellent PowerPC microcontrollers. These chips have been passed down through the corporate generations like a family heirloom—except instead of "Grandma's china," it's intellectual property that went from Motorola → Freescale → NXP. 

*Who says semiconductor mergers aren't romantic?* 💝

## The $40 Board That Was Arduino-Compatible... But Not Really 🤯

In 2017, NXP released the **DEVKIT-MPC5744P**—an absolutely amazing $40 development board with:
- **Arduino UNO R3 footprint compatibility** (so you could use all those shields!)
- Dual-core 200 MHz PowerPC e200z4 processors (locked-step for safety!)
- 2.5 MB of flash, 384 KB of RAM
- Industrial-grade everything

But then... they *didn't release an Arduino-compatible framework*. 

It was like buying a Tesla with a gas cap. The hardware is there, the form factor is perfect, but good luck finding the gasoline equivalent. You're on your own, cowboy! 🤠

## The Activation Key Rebellion 🔑

The official NXP development tools? They required **activation keys**. 

*In 2017.* 

*In the age of open source.*

*When every college grad is using VSCode, JetBrains, or vim.*

"How do we convince a developer to use our tools?" NXP asked themselves.

"We'll make them register!" 🎪

**This did not work.**

## ECLIPSE! (A Horror Story) 😱

The official development environment was **Eclipse**.

Not modern Eclipse. Not VS Code with Java extensions. Not even IntelliJ IDEA. 

**ECLIPSE.**

In 2017. 

In 2025, we're *still* trying to convince people that Eclipse is a viable option. Try telling a fresh CS grad that they should abandon their carefully curated VSCode setup (with 47 extensions, 3 custom themes, and keybindings they've memorized) for... Eclipse. 

*"But it has debugging!"* 

"Yeah, and my terminal has `gdb`, which I can actually use in CI/CD."

## Where's the CLI? Where's the CI/CD? Where's the Open Source Spirit? 🎭

This is where we come in.

**This platform brings the PowerPC VLE ecosystem into the 21st century:**

✅ **Command-line interface** — because automation is a thing  
✅ **CI/CD support** — because `pio run` works in GitHub Actions  
✅ **Open source** — because you shouldn't need a license key to blink an LED  
✅ **Modern tooling** — because your time is valuable  
✅ **PlatformIO integration** — because consistency is king  

No activation keys. No Eclipse. No corporate bureaucracy. Just code.

---

## What Is This Package?

This is a **PlatformIO development platform** for NXP PowerPC microcontrollers, supporting **multiple e200 core variants** (e200z0, e200z4, e200z6) and **instruction set architectures** (VLE and BookE). It provides:

- **Automatic toolchain management** — PlatformIO downloads and manages the PowerPC EABI VLE cross-compilation toolchain
- **Multiple framework support** — Baremetal, FreeRTOS, and yes... **Arduino** (because someone had to do it!)
- **Board support packages** — Pre-configured for multiple NXP PowerPC boards
- **Zero manual setup** — It just works™

---

## Supported Boards

### Industrial Workhorses 🏭

| Board | MCU | CPU | Clock | Flash | RAM | Frameworks |
|-------|-----|-----|-------|-------|-----|------------|
| **DEVKIT-MPC5744P** | MPC-5744P | Dual e200z4 | 160 MHz | 2.5 MB | 384 KB | baremetal, freertos, **arduino** |
| **MPC-5744P** | MPC-5744P | Dual e200z4 | 160 MHz | 2.5 MB | 384 KB | baremetal, freertos |
| **MPC-5748G** | MPC-5748G | e200z4 | 120 MHz | 2 MB | 512 KB | baremetal, freertos |
| **MPC-5748P** | MPC-5748P | e200z4 | 120 MHz | 2 MB | 512 KB | baremetal, freertos |
| **MPC-5775K** | MPC-5775K | e200z4 | 200 MHz | 8 MB | 512 KB | baremetal, freertos |
| **MPC-5646C** | MPC-5646C | e200z4 + e200z0 ⚡ | 120 MHz | 4 MB | 256 KB | baremetal, freertos |
| **MPC-5643L** | MPC-5643L | e200z4 | 120 MHz | 1 MB | 256 KB | baremetal, freertos |

⚡ **MPC-5646C is multicore**: Z4 (e200z4) + Z0 (e200z0) with mixed VLE/BookE support

### The One That Started It All 🎯

**DEVKIT-MPC5744P** deserves special mention:
- **Arduino UNO R3 footprint** — Use Arduino shields!
- **Arduino framework support** — Yes, we actually implemented it!
- CAN, Ethernet (on MAPBGA package), GPIO, SPI, I2C, timers, PWM
- All the industrial-grade features you'd expect from NXP

---

## Architecture Details

This platform supports **multiple PowerPC e200 core variants** and **instruction set architectures**:

### CPU Core Variants

| Core | Instruction Sets | Boards | Description |
|------|------------------|--------|-------------|
| **e200z0** | VLE only | MPC-5646C (Z0 core) | Entry-level core, VLE-only |
| **e200z4** | VLE + BookE | Most boards | Primary core, supports both VLE and BookE |
| **e200z6** | VLE + BookE | (Future/advanced) | Enhanced variant with additional features |

**Most boards use e200z4**, but **MPC-5646C** features a **multicore architecture**:
- **Z4 core (e200z4)**: Supports both VLE and BookE instruction sets
- **Z0 core (e200z0)**: VLE-only, can run in parallel with Z4

### Instruction Set Architectures

#### VLE (Variable Length Encoding) 🔥

**VLE is the primary instruction set** used across all boards:
- **Mixed encoding**: 16-bit and 32-bit instructions for optimal code density
- **ISA**: Power Architecture (Book E compliant)
- **Pipeline**: 4-stage, in-order execution
- **Benefits**:
  - Better code density than pure 32-bit instructions
  - Smaller flash footprint (critical for cost-sensitive designs)
  - Faster instruction fetch (2 instructions per 32-bit word)
  - Backward compatibility with traditional PowerPC tools
- **Features**:
  - Branch prediction
  - Single-cycle multiply
  - Hardware divide
  - Lockstep support (on dual-core variants)

#### BookE (Classic PowerPC) 📚

**BookE is available on e200z4 cores** (like MPC-5646C's Z4 core):
- **Pure 32-bit**: Classic PowerPC instruction set
- **Use case**: Running legacy PowerPC code or when full 32-bit instruction set is needed
- **Mixed mode**: MPC-5646C can run VLE code on Z0 core and BookE code on Z4 core simultaneously!

### Multicore Architecture (MPC-5646C)

The **MPC-5646C** is a special beast—it's a **heterogeneous multicore** microcontroller:

- **Z4 Core (e200z4)**: 
  - Supports both VLE and BookE instruction sets
  - Can run different code modes in different memory regions via MMU
  - Primary core for mixed-mode applications
  
- **Z0 Core (e200z0)**:
  - VLE-only instruction set
  - Runs in parallel with Z4 core
  - Perfect for dedicated real-time tasks

**Mixed VLE/BookE Example**: The platform includes examples showing how to run VLE code on the Z0 core while Z4 core executes BookE code, all on the same chip! Check out `mpc5646c/advanced/mixed-vle-booke-mpc5646c` in the examples repository.

---

## Quick Start

### Installation

Add to your `platformio.ini`:

```ini
[env:devkit-mpc5744p]
platform = nxppowerpc
board = devkit-mpc5744p
framework = arduino
```

Or install directly:

```bash
pio platform install nxppowerpc
```

### Example: Arduino Blink (Finally!)

```ini
[env:devkit-mpc5744p]
platform = nxppowerpc
board = devkit-mpc5744p
framework = arduino
build_flags =
    -Os
    -Wall
```

```cpp
// src/main.cpp
#include <Arduino.h>

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
}
```

### Example: Baremetal Blink

```ini
[env:mpc5748g]
platform = nxppowerpc
board = mpc5748g
framework = baremetal
```

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

### Example: FreeRTOS

```ini
[env:mpc5748g]
platform = nxppowerpc
board = mpc5748g
framework = freertos
```

PlatformIO will automatically download FreeRTOS from the framework registry.

---

## Complete Example `platformio.ini` Configurations

### DEVKIT-MPC5744P with Arduino

```ini
[platformio]
default_envs = devkit-mpc5744p

[env:devkit-mpc5744p]
platform = nxppowerpc
board = devkit-mpc5744p
framework = arduino

; Build options
build_flags =
    -Os                          ; Optimize for size
    -Wall                        ; Enable all warnings
    -Wextra                      ; Extra warnings
    -DDEVKIT_MPC5744P           ; Board define (auto-set)
    -DF_CPU=160000000L          ; 160 MHz (auto-set)

; Monitor options
monitor_speed = 115200
```

### MPC-5748G with Baremetal

```ini
[env:mpc5748g]
platform = nxppowerpc
board = mpc5748g
framework = baremetal

build_flags =
    -Os
    -Wall
    -Wextra
    -DMPC5748G                  ; Board define (auto-set)
    -DF_CPU=120000000L          ; 120 MHz (auto-set)

; Custom linker script (optional)
; -T ${PROJECT_DIR}/linker.ld
```

### MPC-5775K with FreeRTOS

```ini
[env:mpc5775k]
platform = nxppowerpc
board = mpc5775k
framework = freertos

build_flags =
    -Os
    -Wall
    -DMPC5775K
    -DF_CPU=200000000L          ; 200 MHz beast
```

### MPC-5646C (Multicore with Mixed VLE/BookE)

The MPC-5646C features a **heterogeneous multicore architecture** (Z4 + Z0 cores):

```ini
[env:mpc5646c]
platform = nxppowerpc
board = mpc5646c
framework = baremetal

build_flags =
    -Os
    -Wall
    -DMPC5646C
    -DF_CPU=120000000L

; Note: This board supports mixed VLE/BookE operation
; Check examples: mpc5646c/advanced/mixed-vle-booke-mpc5646c
```

---

## Frameworks

### Baremetal 🪨

The minimal framework. You get:
- Automatic startup code (SWT disable, SRAM ECC init, stack setup)
- Automatic linker script detection
- Standard C library linking
- Zero bloat

**Use when**: You need maximum control and minimal overhead.

### FreeRTOS 🎯

Real-time operating system support:
- Task scheduling
- Queues, semaphores, mutexes
- Timers and interrupts
- Memory management

**Use when**: You need multitasking or complex timing requirements.

### Arduino 🎨

Yes, **Arduino support**! Finally!

- `pinMode()`, `digitalWrite()`, `digitalRead()`
- `analogRead()`, `analogWrite()` (PWM)
- `delay()`, `millis()`, `micros()`
- Arduino-style CAN and Ethernet libraries
- All the shields you've been hoarding

**Use when**: You want Arduino compatibility or rapid prototyping.

---

## Build & Run

```bash
# Build
pio run

# Build for specific environment
pio run -e devkit-mpc5744p

# Clean
pio run -t clean

# Size information
pio run -t size

# Generate binary
pio run -t bin
```

---

## Toolchain

The platform automatically downloads and manages the PowerPC EABI VLE toolchain:

- **Compiler**: GCC 4.9.4 (PowerPC EABI VLE)
- **Supported Architectures**: 
  - e200z0 (VLE only)
  - e200z4 (VLE + BookE)
  - e200z6 (VLE + BookE, fallback)
- **Primary Mode**: VLE (Variable Length Encoding)
- **ABI**: EABI
- **Installation**: Automatic via PlatformIO package system

The toolchain automatically selects the appropriate CPU variant based on your board configuration. No manual installation. No Docker required. No activation keys. Just works.

---

## Examples Repository

Check out the examples repository for working code:

**[platform-nxppowerpc-examples](https://github.com/dapperfu/platform-nxppowerpc-examples)**

Examples include:
- Arduino blink, CAN, Ethernet
- FreeRTOS tasks, queues, timers
- Baremetal startup, interrupts, DMA
- Board-specific examples for all supported MCUs

---

## Contributing

We welcome contributions! Areas that need help:
- Additional board support
- Upload/debug tooling (OpenSDA, etc.)
- More Arduino libraries
- Documentation improvements
- CI/CD examples

**Help us make PowerPC development less painful!** 🚀

---

## License

Apache License 2.0 — because open source shouldn't require activation keys.

---

## Resources

- [NXP PowerPC Product Page](https://www.nxp.com/products/processors-and-microcontrollers/power-architecture-processors/powerpc-processors)
- [MPC57xx Reference Manual](https://www.nxp.com/docs/en/reference-manual/MPC57XXRM.pdf)
- [PlatformIO Documentation](https://docs.platformio.org/)
- [Examples Repository](https://github.com/dapperfu/platform-nxppowerpc-examples)

---

## The End (But Actually the Beginning)

No more Eclipse. No more activation keys. No more corporate bureaucracy.

Just `pio run`.

**Welcome to the future of PowerPC development.** ⚡