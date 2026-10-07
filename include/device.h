#pragma once

#include <stdint.h>

// Change this value depending on which device is flashed
constexpr uint16_t DEVICE_ID = 1;

struct Device {
    uint16_t id;
    const char* name;
};

constexpr Device devices[] {
    {1, "Sebastian"},
    {2, "Trevor"},
    {3, "Professor Salemi"}
};

void deviceInit();
void deviceUpdate();