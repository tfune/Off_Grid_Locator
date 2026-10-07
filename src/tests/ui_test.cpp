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
static Member currentMember = SEBASTIAN;

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