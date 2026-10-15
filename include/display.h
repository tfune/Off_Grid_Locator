#pragma once

#include "device.h"

bool displayInit();

void displayStartup();
void displayMemberList(const Device& firstDevice, const Device& secondDevice, const Device& selectedDevice);

// Bearing is absolute geographic direction; arrow angle is relative to device heading
void displayTracking(const Device& selectedDevice, float distance, float bearing, float arrowAngle, bool headingWarning = false, bool withinProximity = false);

void displayLocationUnavailable();
void displayInitializationError();

void displayRefreshBattery();