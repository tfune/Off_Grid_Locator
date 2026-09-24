#include "device.h"
#include "imu.h"
#include <Arduino.h>
#include <Wire.h>

static bool imuInitialized = false;

void deviceInit() {
    Serial.begin(115200);
    Wire.begin();

    Serial.println("Starting IMU Initialization...");

    imuInitialized = imuInit();

    if(imuInitialized == false) {
        Serial.println("IMU Initialization Failed");
        return;
    }

    Serial.println("IMU Initialization Successful");
}

void deviceUpdate() {
    if(imuInitialized) {
        imuUpdate();
    }
}