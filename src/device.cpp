#include "device.h"
#include "imu.h"
#include <Arduino.h>
#include <Wire.h>

static bool imuInitialized = false;

void Device_Init() {
    Serial.begin(115200);
    Wire.begin();

    Serial.println("Starting IMU Initialization...");

    imuInitialized = IMU_Init();

    if(imuInitialized == false) {
    Serial.println("IMU Initialization Failed");
    return;
    }

    Serial.println("IMU Initialization Successful");
}

void Device_Update() {
    if(imuInitialized) {
        IMU_Update();
    }
}