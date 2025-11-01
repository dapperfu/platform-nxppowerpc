/*
 * Ethernet.h - Arduino-style Ethernet library for MPC5744P/MPC5748G
 * Copyright 2014-present PlatformIO
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 */

#ifndef ETHERNET_H
#define ETHERNET_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// IP address structure
typedef struct {
    uint8_t octet[4];
} IPAddress;

// MAC address type
typedef uint8_t MACAddress[6];

// Link status
typedef enum {
    EthernetNoHardware = 0,
    EthernetLinkOff = 1,
    EthernetLinkOn = 2
} EthernetLinkStatus;

// Ethernet class (Arduino-style)
typedef struct {
    // Initialization
    int (*begin)(const uint8_t *mac);
    int (*beginWithIP)(const uint8_t *mac, const uint8_t *ip);
    void (*end)(void);
    
    // Network configuration
    IPAddress (*localIP)(void);
    IPAddress (*subnetMask)(void);
    IPAddress (*gatewayIP)(void);
    IPAddress (*dnsServerIP)(void);
    
    // Status
    EthernetLinkStatus (*linkStatus)(void);
    int (*maintain)(void);  // DHCP maintain
    
    // Low-level functions (for advanced use)
    bool (*setMACAddress)(const uint8_t *mac);
    bool (*setIPAddress)(const uint8_t *ip);
    bool (*setSubnetMask)(const uint8_t *mask);
    bool (*setGatewayIP)(const uint8_t *gateway);
} EthernetClass;

// Global Ethernet instance
extern EthernetClass Ethernet;

#ifdef __cplusplus
}
#endif

#endif // ETHERNET_H

