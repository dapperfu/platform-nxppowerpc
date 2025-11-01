/*
 * main.cpp - Main entry point for Arduino on FreeRTOS
 * Copyright 2014-present PlatformIO
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 */

#include "Arduino.h"
#include "FreeRTOS.h"
#include "task.h"

// Forward declaration from hooks.c
extern void initArduino(void);

/**
 * @brief Main application entry point
 * 
 * Initializes the Arduino framework (which creates the Arduino task)
 * and starts the FreeRTOS scheduler.
 * 
 * @return int Exit code (never returns in embedded systems)
 */
int main(void)
{
    // Initialize hardware (system clocks, peripherals, etc.)
    // This should be done by startup code or user initialization
    
    // Initialize Arduino framework
    // This creates the Arduino task that runs setup() and loop()
    initArduino();
    
    // Start the FreeRTOS scheduler
    // This will start executing tasks (including the Arduino task)
    vTaskStartScheduler();
    
    // If we reach here, the scheduler failed to start
    // This typically means insufficient heap memory or configuration error
    while (1)
    {
        // Halt or error handling
    }
    
    return 0;
}

