#include <Arduino.h>
#include "battery.h"

namespace {
    constexpr unsigned long checkInterval = 30000;

    unsigned long lastCheck = 0;
    float cachedVoltage = 0.0;
    uint8_t cachedLevel = 0;
}

static float batteryReadVoltage()
{
    return analogRead(A6) * 2.0f * 3.6f / 1024.0f;
}

static uint8_t batteryGetLevel(float voltage)
{
    if (voltage >= 4.00) return 4;
    if (voltage >= 3.80) return 3;
    if (voltage >= 3.60) return 2;
    if (voltage >= 3.40) return 1;
    return 0;
}

void batteryInit()
{
    analogReference(AR_DEFAULT);
    analogReadResolution(10);

    cachedVoltage = batteryReadVoltage();
    cachedLevel = batteryGetLevel(cachedVoltage);
    lastCheck = millis();
}

bool batteryUpdate()
{
    const unsigned long now = millis();

    if (now - lastCheck < checkInterval) {
        return false;
    }

    cachedVoltage = batteryReadVoltage();
    cachedLevel = batteryGetLevel(cachedVoltage);
    lastCheck = now;

    return true;
}

float batteryGetVoltage()
{
    return cachedVoltage;
}

uint8_t batteryGetLevel()
{
    return cachedLevel;
}

