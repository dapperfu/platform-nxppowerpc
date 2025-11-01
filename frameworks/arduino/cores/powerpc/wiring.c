/*
 * wiring.c - Core timing functions for Arduino on PowerPC VLE
 * Copyright 2014-present PlatformIO
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 */

#include "Arduino.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

// FreeRTOS tick rate - typically 1000 Hz (1ms per tick)
#define TICKS_PER_MS (configTICK_RATE_HZ / 1000)

// Millisecond counter (FreeRTOS tick count)
unsigned long millis(void)
{
    return (unsigned long)(xTaskGetTickCount() * (1000 / configTICK_RATE_HZ));
}

// Microsecond counter (approximation using FreeRTOS ticks + CPU cycles)
unsigned long micros(void)
{
    unsigned long ms = millis();
    // Approximation: assume 160MHz CPU, 1000Hz tick rate
    // This is a rough approximation - for precise timing, use hardware timer
    unsigned long us_per_tick = 1000000 / configTICK_RATE_HZ;
    return ms * 1000 + (us_per_tick / 2); // Add half tick for better approximation
}

// Delay using FreeRTOS vTaskDelay
void delay(unsigned long ms)
{
    if (ms == 0) {
        return;
    }
    
    // Convert milliseconds to FreeRTOS ticks
    TickType_t ticks = (TickType_t)((ms * configTICK_RATE_HZ) / 1000);
    
    // Ensure at least 1 tick delay
    if (ticks == 0) {
        ticks = 1;
    }
    
    vTaskDelay(ticks);
}

// Delay microseconds (busy wait - use with caution in RTOS environment)
void delayMicroseconds(unsigned int us)
{
    // Busy wait for microsecond delays
    // This is a simple implementation using CPU cycles
    // For 160MHz CPU: 160 cycles per microsecond
    volatile unsigned long cycles = (us * 160); // Approximate cycles needed
    
    // Simple busy loop - compiler should not optimize this away
    while (cycles--) {
        __asm__ volatile ("nop");
    }
}

