#pragma once
#include <stdint.h>

void batteryInit();
bool batteryUpdate();

float batteryGetVoltage();
uint8_t batteryGetLevel();

