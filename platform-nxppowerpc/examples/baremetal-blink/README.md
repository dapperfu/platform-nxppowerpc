# Baremetal Blink Example

Minimal baremetal example for NXP PowerPC VLE microcontrollers (MPC-5748G/MPC-5748P).

## Prerequisites

1. Docker installed and running
2. Docker image `s32ds-power-v1-2:latest` built (see main README)
3. PlatformIO installed

## Project Structure

```
baremetal-blink/
├── platformio.ini    # Project configuration
├── src/
│   └── main.c        # Main application code
└── linker.ld         # Linker script (user-provided)
```

## Notes

This is a minimal example structure. For a complete working project, you will need:

1. **Linker Script** (`linker.ld`): Define memory layout for your target MCU
2. **Startup Code**: Initialize stack pointer, call main, handle interrupts
3. **Application Code**: Your main application

## Building

```bash
cd examples/baremetal-blink
pio run
```

## Next Steps

1. Obtain or create a linker script for your target MCU
2. Add startup code (typically `startup.S` or `startup.c`)
3. Implement your application logic in `main.c`
4. Configure GPIO and peripherals as needed

For production projects, consider:
- Using NXP's SDK or HAL libraries
- Implementing proper startup code with vector tables
- Setting up interrupt handlers
- Configuring clock system
- Initializing peripherals

