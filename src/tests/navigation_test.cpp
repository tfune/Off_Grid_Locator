#include <Arduino.h>
#include <Wire.h>
#include "gps.h"
#include "imu.h"
#include "navigation.h"

static bool imuInitialized = false;
static unsigned long lastPrint = 0;

// Sebastian Test Coordinates: {36.20625, -86.28833}
// Trevor Test Coordinates: {32.65272, -117.08230}
const Coordinates target = {36.20625, -86.28833};

void setup() {
    Serial.begin(115200);

    while(!Serial) {
        delay(10);
    }

    Serial.println("Navigation test started");

    Wire.begin();

    gpsInit();
    imuInitialized = imuInit();

    if(!imuInitialized) {
        Serial.println("IMU initialization failed");
    }
}

void loop() {
    gpsUpdate();

    if(imuInitialized) {
        imuUpdate();
        
    }

    if(millis() - lastPrint >= 1000) {
        GPSData gpsData = gpsGetData();

        if(!gpsData.fix) {
            Serial.println("Waiting for GPS fix...");
            lastPrint = millis();
            return;
        }

        Coordinates current = {
            gpsData.latitude,
            gpsData.longitude
        };

        NavigationResult result =
            calculateNavigation(current, target);

        Serial.print("IMU Accuracy: ");
        Serial.println(imuGetAccuracy(), 1);

        Serial.print("Current Location: ");
        Serial.print(current.latitude, 0);
        Serial.print(", ");
        Serial.println(current.longitude, 0);

        Serial.print("Distance: ");
        Serial.print(result.distance, 1);
        Serial.println(" m");

        Serial.print("Bearing: ");

        if(result.bearingValid) {
            Serial.print(result.bearing, 1);
            Serial.println(" deg");
        }
        else {
            Serial.println("undefined");
        }

        if(imuInitialized) {
            float heading = imuGetHeading();

            Serial.print("Device facing: ");
            Serial.print(heading, 1);
            Serial.println(" deg");

            if(result.bearingValid) {
                float relativeAngle =
                    calculateArrowAngle(result.bearing, heading);

                Serial.print("Target relative to device: ");
                Serial.print(relativeAngle, 1);
                Serial.println(" deg");
            }
        }

        Serial.println("-------------------------");

        lastPrint = millis();
    }
}