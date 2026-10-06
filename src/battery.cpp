#include <Arduino.h>
#include "battery.h"

float batteryReadVoltage()
{
    return analogRead(A6) * 2.0f * 3.6f / 1024.0f;
}

uint8_t batteryGetLevel(float voltage)
{
    if (voltage >= 4.00) return 4;
    if (voltage >= 3.80) return 3;
    if (voltage >= 3.60) return 2;
    if (voltage >= 3.40) return 1;
    return 0;
}

