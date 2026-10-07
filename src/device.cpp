#include <Arduino.h>
#include <Wire.h>
#include "device.h"
#include "imu.h"
#include "gps.h"
#include "display.h"
#include "input.h"
#include "lora.h"
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
                    displayTracking(currentMember, 150.0, 45.0);
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