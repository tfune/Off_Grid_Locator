#include <Arduino.h>
#include <Wire.h>
#include "device.h"
#include "imu.h"
#include "gps.h"
#include "display.h"

static bool displayInitialized = false;
static bool imuInitialized = false;

static unsigned long lastPrint = 0;
static unsigned long lastScreenUpdate = 0;

static int screen = 0;

void deviceInit() {
    Serial.begin(115200);

    while(!Serial) {
        delay(10);
    }

    Serial.println("Device test started");

    Wire.begin();

    displayInitialized = displayInit();
    gpsInit();
    imuInitialized = imuInit();

    Serial.print("Display Initialization: ");
    Serial.println(displayInitialized ? "Successful" : "Failed");

    Serial.print("IMU Initialization: ");
    Serial.println(imuInitialized ? "Successful" : "Failed");

    if(displayInitialized) {
        displayStartup();
    }
}

void deviceUpdate() {
    gpsUpdate();

    if(imuInitialized) {
        imuUpdate();
    }

    GPSData gpsData = gpsGetData();

    if(millis() - lastPrint >= 1000) {
        if(imuInitialized) {
            Serial.print("Heading: ");
            Serial.println(imuGetHeading(), 1);

            Serial.print("IMU Accuracy: ");
            Serial.println(imuGetAccuracy(), 1);
        }

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

    if(displayInitialized && millis() - lastScreenUpdate >= 3000) {
        lastScreenUpdate = millis();

        switch(screen) {
            case 0:
                displayStartup();
                break;

            case 1:
                displayMemberList();
                break;

            case 2:
                displayTracking(125.0, 225.0);
                break;

            case 3:
                displayLocationUnavailable();
                break;

            case 4:
                displayInitializationError();
                break;
        }

        screen++;

        if(screen >= 5) {
            screen = 0;
        }
    }
}