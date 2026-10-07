#include <Arduino.h>
#include <Wire.h>
#include "device.h"
#include "imu.h"
#include "gps.h"
#include "display.h"
#include "input.h"
#include "lora.h"
#include "navigation.h"
#include "target.h"
#include "battery.h"

static unsigned long lastBatteryDisplay = 0;

static bool displayInitialized = false;
static bool imuInitialized = false;

enum Screen {
    STARTUP,
    MEMBER_LIST,
    TRACKING
};

static Screen currentScreen = STARTUP;

static const Coordinates sebastianTarget = {36.20625, -86.28833};
static const Coordinates trevorTarget = {32.65272, -117.08230};
static const Coordinates professorSalemiTarget = {34.05224, -118.24368};

static Device firstDevice;
static Device secondDevice;
static Device selectedDevice;

void deviceInit() {
    if(DEVICE_ID == 1) {
        firstDevice = devices[1];
        secondDevice = devices[2];
    }
    else if(DEVICE_ID == 2) {
        firstDevice = devices[0];
        secondDevice = devices[2];
    }
    else {
        firstDevice = devices[0];
        secondDevice = devices[1];
    }

    selectedDevice = firstDevice;

    Serial.begin(115200);
    Wire.begin();

    batteryInit();
    lastBatteryDisplay = millis();

    displayInitialized = displayInit();
    gpsInit();
    imuInitialized = imuInit();
    inputInit();

    if(displayInitialized) {
        displayStartup();
    }

    loraInit(DEVICE_ID);
}

void deviceUpdate() {
    batteryUpdate();

    const unsigned long now = millis();

    if (displayInitialized && now - lastBatteryDisplay >= 60000) {
        lastBatteryDisplay = now;
        displayRefreshBattery();
        }

    if(displayInitialized && imuInitialized) {
        gpsUpdate();
        imuUpdate();
        inputUpdate();

        int rotation = getRotation();
        bool buttonPress = getButtonPress();

        switch(currentScreen) {
            case STARTUP:
                if(buttonPress) {
                    currentScreen = MEMBER_LIST;
                    displayMemberList(firstDevice, secondDevice, selectedDevice);
                }
                break;

            case MEMBER_LIST:
                if(rotation > 0) {
                    selectedDevice = secondDevice;
                    displayMemberList(firstDevice, secondDevice, selectedDevice);
                }
                else if(rotation < 0) {
                    selectedDevice = firstDevice;
                    displayMemberList(firstDevice, secondDevice, selectedDevice);
                }

                if(buttonPress) {
                    currentScreen = TRACKING;
                }
                break;

            case TRACKING: {
                if(buttonPress) {
                    currentScreen = MEMBER_LIST;
                    displayMemberList(firstDevice, secondDevice, selectedDevice);
                    break;
                }

                GPSData gpsData = gpsGetData();

                if(!gpsData.fix) {
                    displayLocationUnavailable();
                    break;
                }
                
                Coordinates currentCoordinates = {gpsData.latitude, gpsData.longitude};
                Coordinates targetCoordinates;

                if(selectedDevice.id == 1) {
                    targetCoordinates = sebastianTarget;
                }
                else if(selectedDevice.id == 2) {
                    targetCoordinates = trevorTarget;
                }
                else {
                    targetCoordinates = professorSalemiTarget;
                }

                NavigationResult result = calculateNavigation(currentCoordinates, targetCoordinates);

                if(!result.bearingValid) {
                    displayLocationUnavailable();
                    break;
                }

                float heading = imuGetHeading();
                float arrowAngle = calculateArrowAngle(result.bearing, heading);

                displayTracking(selectedDevice, result.distance, result.bearing, arrowAngle);

                break;
            }
        }
    }
}