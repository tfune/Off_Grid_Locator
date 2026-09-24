#include <Arduino.h>
#include <Wire.h>
#include "device.h"
#include "imu.h"
#include "gps.h"

static bool imuInitialized = false;
static unsigned long lastPrint = 0;

void deviceInit() {
    Serial.begin(115200);
    Wire.begin();

    gpsInit();
    imuInitialized = imuInit();
}

void deviceUpdate() {
    gpsUpdate();
    
    if(imuInitialized) {
        imuUpdate();
    }

    if(millis() - lastPrint >= 1000) {
        if(imuInitialized) {
            Serial.print("Heading: ");
            Serial.println(imuGetHeading(), 1);

            Serial.print("IMU Accuracy: ");
            Serial.println(imuGetAccuracy(), 1);
        }
    }
}