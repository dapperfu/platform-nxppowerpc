/**
 * @file main.c
 * @brief FreeRTOS LED blink example for NXP PowerPC VLE
 * 
 * This example demonstrates a simple LED blink using FreeRTOS tasks.
 * The LED blinks at 1-second intervals using a FreeRTOS task with delays.
 * 
 * @note This is a template that requires:
 * - LED GPIO pin configuration (define LED_PIN and LED_PORT)
 * - GPIO initialization function
 * - System clock initialization
 * - Startup code and linker script
 */

#include "FreeRTOS.h"
#include "task.h"

/* LED configuration - adjust these for your hardware */
#ifndef LED_PORT
#define LED_PORT    (void *)0  /* GPIO port base address - adjust for your board */
#endif
#ifndef LED_PIN
#define LED_PIN     0           /* GPIO pin number - adjust for your board */
#endif

/* Forward declarations */
void LED_Init(void);
void LED_Toggle(void);
void vBlinkTask(void *pvParameters);

/**
 * @brief Initialize the LED GPIO pin
 * 
 * This function should configure the GPIO pin connected to the LED
 * as an output. You must implement this based on your hardware setup.
 */
void LED_Init(void)
{
    /* TODO: Implement GPIO initialization
     * Example pseudo-code:
     * - Enable GPIO peripheral clock
     * - Configure pin as output
     * - Set initial state (LED off)
     */
}

/**
 * @brief Toggle the LED state
 * 
 * This function toggles the LED on/off. You must implement this
 * based on your hardware GPIO register layout.
 */
void LED_Toggle(void)
{
    /* TODO: Implement LED toggle
     * Example pseudo-code:
     * - Read current GPIO state
     * - Toggle the LED pin
     * - Write back to GPIO register
     */
}

/**
 * @brief FreeRTOS task to blink the LED
 * 
 * This task runs in a loop, toggling the LED and delaying for 1 second
 * (1000 milliseconds / 1000 ticks per second = 1000 ticks).
 * 
 * @param pvParameters Task parameters (unused)
 */
void vBlinkTask(void *pvParameters)
{
    (void)pvParameters;  /* Suppress unused parameter warning */
    
    for (;;)
    {
        LED_Toggle();
        /* Delay for 1000 milliseconds (1000 ticks at 1000 Hz tick rate) */
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/**
 * @brief Main application entry point
 * 
 * Initializes hardware, creates the blink task, and starts the
 * FreeRTOS scheduler.
 * 
 * @return int Exit code (never returns in embedded systems)
 */
int main(void)
{
    /* TODO: Initialize system clocks, peripherals, etc. */
    
    /* Initialize LED GPIO */
    LED_Init();
    
    /* Create the LED blink task
     * Parameters:
     *   - Task name: "Blink"
     *   - Stack size: 128 words (512 bytes on 32-bit system)
     *   - Parameters: NULL (none)
     *   - Priority: 1 (above idle task)
     */
    xTaskCreate(
        vBlinkTask,          /* Task function */
        "Blink",            /* Task name */
        128,                /* Stack size in words */
        NULL,               /* Parameters */
        1,                  /* Priority */
        NULL                /* Task handle (not needed) */
    );
    
    /* Start the FreeRTOS scheduler */
    vTaskStartScheduler();
    
    /* If we reach here, the scheduler failed to start */
    /* This typically means insufficient heap memory */
    while (1)
    {
        /* Halt or error handling */
    }
    
    return 0;
}


