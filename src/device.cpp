#include <Arduino.h>
#include <Wire.h>
#include "device.h"
#include "imu.h"
#include "gps.h"
#include "display.h"

static bool imuInitialized = false;
static unsigned long lastPrint = 0;

void deviceInit() {
    Serial.begin(115200);

    while(!Serial) {
        delay(10);
    }

    Wire.begin();

    if(!displayInit()) {
        Serial.println("Display Initialization Failed");
        return;
    }

    Serial.println("Display Initialization Successful");

    gpsInit();

    imuInitialized = imuInit();

    if(!imuInitialized) {
        Serial.println("IMU Initialization Failed");
        return;
    }

    Serial.println("IMU Initialization Successful");

    displayStartup();
}

void deviceUpdate() {
    gpsUpdate();
    
    if(imuInitialized) {
        imuUpdate();

        if(millis() - lastPrint >= 1000) {
            Serial.print("Heading: ");
            Serial.println(imuGetHeading(), 1);
            lastPrint = millis();
        }
    }
}