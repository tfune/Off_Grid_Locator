#pragma once

#include <stdint.h>

// Change this value depending on which device is flashed
// 1: Sebastian, 2: Trevor, 3: Professor Salemi
constexpr uint16_t DEVICE_ID = 1;

// Identifies a group member and their assigned device
struct Device {
    uint16_t id;
    const char* name;
};

// Known devices in the tracking group
constexpr Device devices[] {
    {1, "Sebastian"},
    {2, "Trevor"},
    {3, "Professor Salemi"}
};

void deviceInit();
void deviceUpdate();