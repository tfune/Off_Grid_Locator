#pragma once

#include "device.h"

bool displayInit();
void displayStartup();

void displayMemberList(const Device& firstDevice, const Device& secondDevice, const Device& selectedDevice);
void displayTracking(const Device& selectedDevice, float distance, float bearing, float arrowAngle);
void displayLocationUnavailable();
void displayInitializationError();
void displayRefreshBattery();