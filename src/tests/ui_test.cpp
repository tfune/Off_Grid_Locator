#include <Arduino.h>
#include <Wire.h>
#include "display.h"
#include "input.h"

static bool displayInitialized = false;

enum Screen {
    STARTUP,
    MEMBER_LIST,
    TRACKING
};

static Screen currentScreen = STARTUP;
static Device firstDevice = devices[1];
static Device secondDevice = devices[2];
static Device selectedDevice = firstDevice;

void setup() {
    Serial.begin(115200);

    while(!Serial) {
        delay(10);
    }

    Serial.println("UI integration test started");

    Wire.begin();

    displayInitialized = displayInit();
    inputInit();

    if(displayInitialized) {
        displayStartup();
    }
}

void loop() {
    if(!displayInitialized) {
        return;
    }

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
                displayTracking(selectedDevice, 150.0, 45.0, 45.0);
            }
            break;

        case TRACKING:
            if(buttonPress) {
                currentScreen = MEMBER_LIST;
                displayMemberList(firstDevice, secondDevice, selectedDevice);
            }
            break;
    }
}