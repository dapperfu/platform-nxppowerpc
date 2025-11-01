/*
 * wiring_pulse.c - Pulse measurement functions for Arduino on PowerPC VLE
 * Copyright 2014-present PlatformIO
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 */

#include "Arduino.h"
#include "FreeRTOS.h"
#include "task.h"

unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout)
{
    return pulseInLong(pin, state, timeout);
}

unsigned long pulseInLong(uint8_t pin, uint8_t state, unsigned long timeout)
{
    unsigned long start_micros = micros();
    unsigned long timeout_micros = start_micros + timeout;
    
    // Wait for initial state (opposite of what we're measuring)
    while (digitalRead(pin) == state) {
        if (timeout > 0 && micros() >= timeout_micros) {
            return 0; // Timeout
        }
    }
    
    // Wait for pulse to start (edge)
    while (digitalRead(pin) != state) {
        if (timeout > 0 && micros() >= timeout_micros) {
            return 0; // Timeout
        }
    }
    
    // Pulse started - measure duration
    unsigned long start = micros();
    
    // Wait for pulse to end
    while (digitalRead(pin) == state) {
        if (timeout > 0 && micros() >= timeout_micros) {
            return 0; // Timeout
        }
    }
    
    unsigned long end = micros();
    
    if (timeout > 0 && end >= timeout_micros) {
        return 0; // Timeout
    }
    
    return end - start;
}

