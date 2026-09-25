#include <Arduino.h>
#include <Wire.h>
#include "imu.h"
#include "gps.h"
#include "display.h"

static bool imuInitialized = false;

void deviceInit() {
    Serial.begin(115200);
    Wire.begin();

    displayInit();
    gpsInit();
    imuInitialized = imuInit();

    displayStartup();
}

void deviceUpdate() {
    gpsUpdate();
    
    if(imuInitialized) {
        imuUpdate();
    }
}