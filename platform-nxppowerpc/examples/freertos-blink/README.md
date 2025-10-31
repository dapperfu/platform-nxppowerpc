# FreeRTOS Blink Example for NXP PowerPC VLE

This example demonstrates a simple LED blink application using FreeRTOS on NXP PowerPC VLE microcontrollers (MPC-5748G and MPC-5748P).

## Description

The application creates a FreeRTOS task that blinks an LED at 1-second intervals. This demonstrates:
- FreeRTOS task creation
- Task delays using `vTaskDelay`
- Basic GPIO control (LED toggling)

## Hardware Setup

The example assumes an LED is connected to a GPIO pin. You may need to:
1. Configure the LED pin based on your hardware
2. Provide appropriate startup code and linker script
3. Initialize the GPIO peripheral before starting the FreeRTOS scheduler

## Building

```bash
pio run -e mpc5748g
# or
pio run -e mpc5748p
```

## Configuration

The example uses standard FreeRTOS configuration:
- Tick rate: 1000 Hz
- Max priorities: 5
- Minimal stack size: 128 bytes

You can modify these values in `platformio.ini` via build flags.

## Important Notes

- You must provide a linker script (linker.ld) for your specific board
- You must provide startup code (startup.S or startup.c) 
- GPIO initialization code must be added to match your hardware setup
- The LED pin definition (LED_PIN, LED_PORT) must be configured for your board


