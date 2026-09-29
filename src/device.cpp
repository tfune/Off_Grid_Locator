#include <Arduino.h>
#include <Wire.h>
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

void deviceInit() {
    Serial.begin(115200);
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
}