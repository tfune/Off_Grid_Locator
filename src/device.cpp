#include "device.h"
#include "imu.h"
#include <Arduino.h>
#include <Wire.h>

void Device_Init() {
    Serial.begin(115200);
    Wire.begin();

    Serial.println("Starting IMU Initialization...");

    if(IMU_Init() == false) {
    Serial.println("IMU Initialization Failed");
    return;
    }

    Serial.println("IMU Initialization Successful");
}

void Device_Update() {
    IMU_Update();
}