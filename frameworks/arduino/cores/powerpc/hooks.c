/*
 * hooks.c - FreeRTOS hooks for Arduino integration
 * Copyright 2014-present PlatformIO
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 */

#include "Arduino.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdint.h>

// Arduino setup() and loop() function declarations
// These are weak symbols that can be overridden by user code
void __attribute__((weak)) setup(void);
void __attribute__((weak)) loop(void);

// Main Arduino task function
static void arduinoTask(void *pvParameters)
{
    // Call user's setup() function once
    setup();
    
    // Call user's loop() function repeatedly
    for (;;) {
        loop();
        
        // Yield to other tasks
        taskYIELD();
    }
}

// FreeRTOS idle hook - can be used for low-power modes
void vApplicationIdleHook(void)
{
    // Can add low-power mode here if needed
}

// FreeRTOS tick hook - called every tick
void vApplicationTickHook(void)
{
    // Can be used for periodic tasks
}

// FreeRTOS stack overflow hook
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    // Handle stack overflow
    // In production, this should log or handle the error
    for (;;) {
        // Halt or reset
    }
}

// FreeRTOS malloc failed hook
void vApplicationMallocFailedHook(void)
{
    // Handle malloc failure
    for (;;) {
        // Halt or reset
    }
}

// Initialize Arduino framework (called from main or startup code)
// Note: vTaskStartScheduler() must be called from main() after this function
void initArduino(void)
{
    // Create main Arduino task
    xTaskCreate(
        arduinoTask,
        "Arduino",
        configMINIMAL_STACK_SIZE * 4,  // Stack size (adjust as needed)
        NULL,
        tskIDLE_PRIORITY + 1,          // Priority
        NULL
    );
    
    // Note: vTaskStartScheduler() should be called from main()
    // See main.cpp in the Arduino core
}

// setup() and loop() are declared as weak symbols above
// Users should provide their own implementations in their code

