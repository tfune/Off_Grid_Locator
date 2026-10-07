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
static Member currentMember = SEBASTIAN;

static const Coordinates sebastianTarget = {36.20625, -86.28833};
static const Coordinates professorSalemiTarget = {32.65272, -117.08230};

void deviceInit(uint16_t deviceAddress) {
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
    loraInit(deviceAddress);
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
                    displayMemberList(currentMember);
                }
                break;

            case MEMBER_LIST:
                if(rotation > 0) {
                    currentMember = PROFESSOR_SALEMI;
                    displayMemberList(currentMember);
                }
                else if(rotation < 0) {
                    currentMember = SEBASTIAN;
                    displayMemberList(currentMember);
                }

                if(buttonPress) {
                    currentScreen = TRACKING;
                }
                break;

            case TRACKING: {
                if(buttonPress) {
                    currentScreen = MEMBER_LIST;
                    displayMemberList(currentMember);
                    break;
                }

                GPSData gpsData = gpsGetData();

                if(!gpsData.fix) {
                    displayLocationUnavailable();
                    break;
                }
                
                Coordinates currentCoordinates = {gpsData.latitude, gpsData.longitude};
                Coordinates targetCoordinates;

                if(currentMember == SEBASTIAN) {
                    targetCoordinates = sebastianTarget;
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

                displayTracking(currentMember, result.distance, result.bearing, arrowAngle);

                break;
            }
        }
    }
}