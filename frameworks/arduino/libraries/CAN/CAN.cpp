/*
 * CAN.cpp - Arduino-style CAN library implementation for MPC5744P
 * Copyright 2014-present PlatformIO
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 */

#include "CAN.h"
#include "MPC5744P.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <stdint.h>
#include <stdbool.h>

// CAN module selection (CAN_1 for DEVKIT-MPC5744P on PA14/PA15)
// According to DEVKIT-MPC5744P pinout:
// - J2-18: PA14 (CAN1_TXD)
// - J2-20: PA15 (CAN1_RXD)
#define USE_CAN_MODULE CAN_1
#define RX_MB_INDEX 4
#define TX_MB_INDEX 0

// State variables
static bool can_initialized = false;
static uint32_t can_baudrate = 500000; // Default 500 kbps
static SemaphoreHandle_t can_mutex = NULL;
static CANMessage rx_message;
static bool rx_available = false;

// Calculate CAN bit timing from baudrate
static uint32_t calculateCANBitTiming(uint32_t baudrate)
{
    // For 40 MHz CAN clock source and 500 kbps:
    // PRESDIV+1 = 40MHz / 8MHz = 5, so PRESDIV = 4
    // PSEG2 = 3, PSEG1 = 3, PROPSEG = 6, RJW = 1
    // CTRL1.R = 0x04DB0086
    
    // Simplified: for now, use fixed 500kbps timing
    // Full implementation would calculate based on baudrate
    if (baudrate == 500000) {
        return 0x04DB0086;
    }
    
    // Default to 500kbps timing
    return 0x04DB0086;
}

static bool CAN_begin(uint32_t baudrate)
{
    if (can_initialized) {
        return true; // Already initialized
    }
    
    can_baudrate = baudrate;
    
    // Create mutex for thread safety
    can_mutex = xSemaphoreCreateMutex();
    if (can_mutex == NULL) {
        return false;
    }
    
    // Disable module before selecting clock source
    USE_CAN_MODULE.MCR.B.MDIS = 1;
    
    // Clock Source = oscillator clock (40 MHz)
    USE_CAN_MODULE.CTRL1.B.CLKSRC = 0;
    
    // Enable module for config (Sets FRZ, HALT)
    USE_CAN_MODULE.MCR.B.MDIS = 0;
    
    // Wait for freeze acknowledge
    while (!USE_CAN_MODULE.MCR.B.FRZACK) {
        // Wait
    }
    
    // Configure CAN bit timing
    uint32_t timing = calculateCANBitTiming(baudrate);
    USE_CAN_MODULE.CTRL1.R = timing;
    
    // Initialize all message buffers to inactive
    for (uint8_t i = 0; i < 64; i++) {
        USE_CAN_MODULE.MB[i].CS.B.CODE = 0;
    }
    
    // Configure receive message buffer (MB 4)
    USE_CAN_MODULE.MB[RX_MB_INDEX].CS.B.IDE = 0;      // Standard ID
    USE_CAN_MODULE.MB[RX_MB_INDEX].ID.B.ID_STD = 0;  // Accept any ID (filter will set)
    USE_CAN_MODULE.MB[RX_MB_INDEX].CS.B.CODE = 4;    // RX EMPTY
    USE_CAN_MODULE.RXMGMASK.R = 0x1FFFFFFF;          // Global acceptance mask
    
    // Configure transmit message buffer (MB 0)
    USE_CAN_MODULE.MB[TX_MB_INDEX].CS.B.CODE = 8;     // TX INACTIVE
    
    // Configure CAN pins (DEVKIT-MPC5744P uses CAN_1 on PA14/PA15)
    // CAN1_TX: PA14 (J2-18)
    // PA14 pad index is 14 (from project.h: #define PA14 14)
    SIUL2.MSCR[14].B.SSS = 1;      // CAN1_TX signal source (SSS=1 for CAN1_TX)
    SIUL2.MSCR[14].B.OBE = 1;      // Output buffer enable
    SIUL2.MSCR[14].B.SRC = 3;      // Maximum slew rate
    
    // CAN1_RX: PA15 (J2-20)
    // PA15 pad index is 15
    SIUL2.MSCR[15].B.IBE = 1;      // Input buffer enable
    SIUL2.IMCR[33].B.SSS = 0b0001; // CAN1_RX input mux (IMCR[33] for CAN1_RX)
    
    // Negate FlexCAN halt state
    USE_CAN_MODULE.MCR.R = 0x0000003F;
    
    // Wait for ready
    while (USE_CAN_MODULE.MCR.B.FRZACK & USE_CAN_MODULE.MCR.B.NOTRDY) {
        // Wait
    }
    
    can_initialized = true;
    return true;
}

static void CAN_end(void)
{
    if (!can_initialized) {
        return;
    }
    
    // Disable CAN module
    USE_CAN_MODULE.MCR.B.MDIS = 1;
    
    // Delete mutex
    if (can_mutex != NULL) {
        vSemaphoreDelete(can_mutex);
        can_mutex = NULL;
    }
    
    can_initialized = false;
    rx_available = false;
}

static bool CAN_write(uint32_t id, const uint8_t *data, uint8_t length)
{
    if (!can_initialized || length > 8 || data == NULL) {
        return false;
    }
    
    if (xSemaphoreTake(can_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return false;
    }
    
    // Wait for transmit buffer to be inactive
    while (USE_CAN_MODULE.MB[TX_MB_INDEX].CS.B.CODE != 8) {
        // Wait for buffer to become inactive
    }
    
    // Configure message buffer for transmission
    USE_CAN_MODULE.MB[TX_MB_INDEX].CS.B.IDE = 0;        // Standard ID
    USE_CAN_MODULE.MB[TX_MB_INDEX].ID.B.ID_STD = id;    // Set ID
    USE_CAN_MODULE.MB[TX_MB_INDEX].CS.B.RTR = 0;        // Data frame
    USE_CAN_MODULE.MB[TX_MB_INDEX].CS.B.DLC = length; // Set length
    
    // Copy data
    for (uint8_t i = 0; i < length && i < 8; i++) {
        USE_CAN_MODULE.MB[TX_MB_INDEX].DATA.B[i] = data[i];
    }
    
    // Set SRR (not required for standard frame, but set for consistency)
    USE_CAN_MODULE.MB[TX_MB_INDEX].CS.B.SRR = 1;
    
    // Trigger transmission
    USE_CAN_MODULE.MB[TX_MB_INDEX].CS.B.CODE = 0xC; // TX DATA
    
    xSemaphoreGive(can_mutex);
    
    // Wait for transmission to complete
    while (USE_CAN_MODULE.MB[TX_MB_INDEX].CS.B.CODE != 8) {
        // Wait
    }
    
    return true;
}

static bool CAN_read(uint32_t *id, uint8_t *data, uint8_t *length)
{
    if (!can_initialized || id == NULL || data == NULL || length == NULL) {
        return false;
    }
    
    if (xSemaphoreTake(can_mutex, pdMS_TO_TICKS(10)) != pdTRUE) {
        return false;
    }
    
    // Check if message is available (CODE != 4 means message present)
    uint8_t code = USE_CAN_MODULE.MB[RX_MB_INDEX].CS.B.CODE;
    if (code != 4) {
        // Message received - read it
        *id = USE_CAN_MODULE.MB[RX_MB_INDEX].ID.B.ID_STD;
        *length = USE_CAN_MODULE.MB[RX_MB_INDEX].CS.B.DLC;
        
        // Copy data
        for (uint8_t i = 0; i < *length && i < 8; i++) {
            data[i] = USE_CAN_MODULE.MB[RX_MB_INDEX].DATA.B[i];
        }
        
        // Read TIMER to unlock message buffers (required by hardware)
        volatile uint32_t dummy = USE_CAN_MODULE.TIMER.R;
        (void)dummy; // Suppress unused variable warning
        
        // Clear interrupt flag for message buffer 4
        USE_CAN_MODULE.IFLAG1.R = 0x00000010; // Clear bit 4 (MB4 interrupt flag)
        
        // Re-arm message buffer for next reception (RX EMPTY = 4)
        USE_CAN_MODULE.MB[RX_MB_INDEX].CS.B.CODE = 4; // RX EMPTY
        
        rx_available = false;
        
        xSemaphoreGive(can_mutex);
        return true;
    }
    
    xSemaphoreGive(can_mutex);
    return false;
}

static bool CAN_available(void)
{
    if (!can_initialized) {
        return false;
    }
    
    // Check if receive buffer has a message
    // CODE 4 = RX EMPTY, so any other value means message is present
    // Also check IFLAG interrupt flag for more reliable detection
    // BUF4TO1I: bits 7-4 represent MB4-MB1 interrupt flags
    // Value 8 (0b1000) means MB4 interrupt is set
    uint8_t code = USE_CAN_MODULE.MB[RX_MB_INDEX].CS.B.CODE;
    uint8_t iflag = USE_CAN_MODULE.IFLAG1.B.BUF4TO1I;
    
    // Check interrupt flag first (more reliable for interrupt-driven systems)
    // Bit 3 (0x08) of BUF4TO1I indicates MB4 interrupt
    if ((iflag & 0x08) != 0 && code != 4) {
        rx_available = true;
        return true;
    }
    
    // Fallback: check CODE directly (for polling without interrupts)
    if (code != 4) {
        rx_available = true;
        return true;
    }
    
    return false;
}

static bool CAN_setFilter(uint32_t id, uint32_t mask)
{
    if (!can_initialized) {
        return false;
    }
    
    if (xSemaphoreTake(can_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return false;
    }
    
    // Configure receive message buffer filter
    USE_CAN_MODULE.MB[RX_MB_INDEX].ID.B.ID_STD = id;
    USE_CAN_MODULE.RXMGMASK.R = mask;
    
    xSemaphoreGive(can_mutex);
    return true;
}

// CAN instance
CANClass CAN = {
    .begin = CAN_begin,
    .end = CAN_end,
    .write = CAN_write,
    .read = CAN_read,
    .available = CAN_available,
    .setFilter = CAN_setFilter
};

