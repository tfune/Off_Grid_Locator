#include <Arduino.h>
#include <Wire.h>
#include "imu.h"
#include "gps.h"
#include "display.h"

static bool displayInitialized = false;
static bool imuInitialized = false;

void deviceInit() {
    Serial.begin(115200);
    Wire.begin();

    displayInitialized = displayInit();
    gpsInit();
    imuInitialized = imuInit();

    if(displayInitialized) {
        displayStartup();
    }
}

void deviceUpdate() {
    if(displayInitialized && imuInitialized) {
        gpsUpdate();
        imuUpdate();
    }
}