#include <Arduino.h>
#include <Wire.h>
#include "imu.h"

static bool imuInitialized = false;
static unsigned long lastPrint = 0;

void setup() {
    Serial.begin(115200);

    while(!Serial) {
        delay(10);
    }

    Serial.println("IMU test started");

    Wire.begin();

    imuInitialized = imuInit();

    if(!imuInitialized) {
        Serial.println("IMU initialization failed");
    }
}

void loop() {
    if(imuInitialized) {
        imuUpdate();

        if(millis() - lastPrint >= 1000) {
            Serial.print("Heading: ");
            Serial.println(imuGetHeading(), 1);

            Serial.print("IMU Accuracy: ");
            Serial.println(imuGetAccuracy(), 1);

            Serial.println("-------------------------");

            lastPrint = millis();
        }
    }
}