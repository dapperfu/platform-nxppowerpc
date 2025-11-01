/*
 * CAN.h - Arduino-style CAN library for MPC5744P
 * Copyright 2014-present PlatformIO
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 */

#ifndef CAN_H
#define CAN_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// CAN message structure
typedef struct {
    uint32_t id;
    uint8_t length;
    uint8_t data[8];
    bool extended;
    bool remote;
} CANMessage;

// CAN class (Arduino-style)
typedef struct {
    bool (*begin)(uint32_t baudrate);
    void (*end)(void);
    bool (*write)(uint32_t id, const uint8_t *data, uint8_t length);
    bool (*read)(uint32_t *id, uint8_t *data, uint8_t *length);
    bool (*available)(void);
    bool (*setFilter)(uint32_t id, uint32_t mask);
} CANClass;

// Global CAN instance
extern CANClass CAN;

#ifdef __cplusplus
}
#endif

#endif // CAN_H

