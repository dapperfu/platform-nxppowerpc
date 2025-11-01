/*
 * wiring_digital.c - Digital I/O functions for Arduino on PowerPC VLE
 * Copyright 2014-present PlatformIO
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 */

#include "Arduino.h"
#include "MPC5744P.h"
#include <stdint.h>

// Map Arduino pin numbers to MPC5744P port/pin
// DEVKIT-MPC5744P uses SIUL2 for GPIO
// Pin mapping follows Arduino UNO R3 footprint on devkit

// For now, implement basic pin mapping
// Users may need to customize based on their board layout
static uint8_t pin_modes[128] = {0}; // Track pin modes

void pinMode(uint8_t pin, uint8_t mode)
{
    if (pin >= 128) {
        return; // Invalid pin
    }
    
    pin_modes[pin] = mode;
    
    // Map pin to SIUL2 pad
    // This is a simplified implementation
    // Full implementation would map to actual SIUL2.MSCR registers
    
    uint32_t pad_idx = pin; // Simplified mapping
    
    if (pad_idx < 144) { // MPC5744P has up to 144 pads
        // Configure pad based on mode
        switch (mode) {
            case INPUT:
                SIUL2.MSCR[pad_idx].B.SSS = 0;   // Signal source select = GPIO (0 = GPIO)
                SIUL2.MSCR[pad_idx].B.IBE = 1;  // Input buffer enable
                SIUL2.MSCR[pad_idx].B.OBE = 0;  // Output buffer disable
                SIUL2.MSCR[pad_idx].B.SRC = 0;  // Default slew rate
                break;
                
            case INPUT_PULLUP:
                SIUL2.MSCR[pad_idx].B.SSS = 0;   // GPIO mode
                SIUL2.MSCR[pad_idx].B.IBE = 1;
                SIUL2.MSCR[pad_idx].B.OBE = 0;
                SIUL2.MSCR[pad_idx].B.SRC = 0;
                // Set pull-up (PSPCR register)
                SIUL2.PSPCR[pad_idx].B.PE = 1;   // Pull enable
                SIUL2.PSPCR[pad_idx].B.PS = 1;   // Pull-up (1 = pull-up, 0 = pull-down)
                break;
                
            case INPUT_PULLDOWN:
                SIUL2.MSCR[pad_idx].B.SSS = 0;   // GPIO mode
                SIUL2.MSCR[pad_idx].B.IBE = 1;
                SIUL2.MSCR[pad_idx].B.OBE = 0;
                SIUL2.MSCR[pad_idx].B.SRC = 0;
                // Set pull-down
                SIUL2.PSPCR[pad_idx].B.PE = 1;   // Pull enable
                SIUL2.PSPCR[pad_idx].B.PS = 0;   // Pull-down
                break;
                
            case OUTPUT:
                SIUL2.MSCR[pad_idx].B.SSS = 0;   // Signal source select = GPIO (0 = GPIO)
                SIUL2.MSCR[pad_idx].B.IBE = 0;  // Input buffer disable
                SIUL2.MSCR[pad_idx].B.OBE = 1;  // Output buffer enable
                SIUL2.MSCR[pad_idx].B.SRC = 3;  // Maximum slew rate (3 = max drive, no slew control)
                break;
        }
    }
}

void digitalWrite(uint8_t pin, uint8_t val)
{
    if (pin >= 128) {
        return;
    }
    
    uint32_t pad_idx = pin; // Simplified mapping
    
    if (pad_idx < 144) {
        // Set or clear GPIO output
        // Use PDO field (bit 0) for single-bit GPIO output
        if (val == HIGH) {
            SIUL2.GPDO[pad_idx].B.PDO = 1;
        } else {
            SIUL2.GPDO[pad_idx].B.PDO = 0;
        }
    }
}

int digitalRead(uint8_t pin)
{
    if (pin >= 128) {
        return LOW;
    }
    
    uint32_t pad_idx = pin; // Simplified mapping
    
    if (pad_idx < 144) {
        // Read GPIO input
        return (SIUL2.GPDI[pad_idx].B.PDI != 0) ? HIGH : LOW;
    }
    
    return LOW;
}

