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

    if(!imuInitialized) {
        Serial.println("IMU Initialization Failed");
        return;
    }

    Serial.println("IMU Initialization Successful");
}

void deviceUpdate() {
    gpsUpdate();
    GPSData gpsData = gpsGetData();
    
    if(imuInitialized) {
        imuUpdate();
    }

    if(millis() - lastPrint >= 1000) {
        if(imuInitialized) {
            Serial.print("Heading: ");
            Serial.println(imuGetHeading(), 1);

            Serial.print("IMU Accuracy: ");
            Serial.println(imuGetAccuracy(), 1);
                    Serial.print("GPS Fix: ");
            Serial.println(gpsData.fix ? "Yes" : "No");

            Serial.print("Latitude: ");
            Serial.println(gpsData.latitude, 6);

            Serial.print("Longitude: ");
            Serial.println(gpsData.longitude, 6);

            Serial.print("Time: ");
            Serial.println(gpsData.time);
            Serial.println("-------------------------");
            lastPrint = millis();
        }
    }
}