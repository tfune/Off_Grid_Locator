#include <Arduino.h>
#include <Wire.h>
#include "device.h"
#include "imu.h"
#include "gps.h"
#include "display.h"
#include "input.h"

static bool displayInitialized = false;
static bool imuInitialized = false;

enum Screen {
    STARTUP,
    MEMBER_LIST,
    TRACKING
};

static Screen currentScreen = STARTUP;
static int currentMember = 0;

static unsigned long lastPrint = 0;

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
    inputInit();

    if(displayInitialized) {
        displayStartup();
    }
}

void deviceUpdate() {
    gpsUpdate();
    GPSData gpsData = gpsGetData();

    if(imuInitialized) {
        imuUpdate();
    }

    inputUpdate();

    int rotation = getRotation();
    bool buttonPress = getButtonPress();

    if(displayInitialized) {
        switch(currentScreen) {
            case STARTUP:
                if(buttonPress) {
                    currentScreen = MEMBER_LIST;
                    displayMemberList(currentMember);
                }
                break;

            case MEMBER_LIST:
                if(rotation > 0) {
                    currentMember = 1;
                    displayMemberList(currentMember);
                }
                else if(rotation < 0) {
                    currentMember = 0;
                    displayMemberList(currentMember);
                }

                if(buttonPress) {
                    currentScreen = TRACKING;
                    displayTracking(150.0, 45.0);
                }

                break;

            case TRACKING:
                if(buttonPress) {
                    currentScreen = MEMBER_LIST;
                    displayMemberList(currentMember);
                }
                
                break;
        }
    }

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
}