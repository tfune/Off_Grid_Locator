#include "device.h"
#include "imu.h"
#include "gps.h"
#include <Arduino.h>
#include <Wire.h>

static bool imuInitialized = false;
static unsigned long lastPrint = 0;

void deviceInit() {
    Serial.begin(115200);
    Wire.begin();

    gpsInit();

    Serial.println("Starting IMU Initialization...");

    imuInitialized = imuInit();

    if(imuInitialized == false) {
        Serial.println("IMU Initialization Failed");
        return;
    }

    Serial.println("IMU Initialization Successful");
}

void deviceUpdate() {

    gpsUpdate();
    
    if(imuInitialized) {
        imuUpdate();
    }
}