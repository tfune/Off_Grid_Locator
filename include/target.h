#pragma once
#include <stdint.h>
#include "navigation.h"

bool targetSet(uint16_t deviceId, const Coordinates& coordinates, unsigned long timeStamp);
bool targetGet(uint16_t deviceId, Coordinates& coordinates, unsigned long now, unsigned long maxAge);
