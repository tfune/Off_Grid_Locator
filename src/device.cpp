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
    Wire.begin();

    displayInitialized = displayInit();
    gpsInit();
    imuInitialized = imuInit();

    if(!imuInitialized) {
        Serial.println("IMU Initialization Failed");
        return;
    }

    Serial.println("IMU Initialization Successful");

    if(displayInitialized) {
        displayStartup();
    }
}

void deviceUpdate() {
    if(imuInitialized && displayInitialized) {
        gpsUpdate();
        imuUpdate();

        GPSData gpsData = gpsGetData();

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

            if(millis() - lastScreenUpdate >= 3000) {
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
    }
}