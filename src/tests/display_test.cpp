#include <Arduino.h>
#include <Wire.h>
#include "display.h"

static bool displayInitialized = false;
static unsigned long lastScreenChange = 0;
static int currentScreen = 0;

void setup() {
    Serial.begin(115200);

    while(!Serial) {
        delay(10);
    }

    Serial.println("Display test started");

    Wire.begin();

    displayInitialized = displayInit();

    if(!displayInitialized) {
        Serial.println("Display initialization failed");
        return;
    }

    displayStartup();
}

void loop() {
    if(!displayInitialized) {
        return;
    }

    if(millis() - lastScreenChange >= 3000) {
        currentScreen++;

        if(currentScreen >4) {
            currentScreen = 0;
        }

        switch(currentScreen) {
            case 0:
                displayStartup();
                break;

            case 1:
                displayMemberList(SEBASTIAN);
                break;

            case 2:
                displayTracking(SEBASTIAN, 125.0, 225.0, 225.0);
                break;

            case 3:
                displayLocationUnavailable();
                break;

            case 4:
                displayInitializationError();
                break;
        }

        lastScreenChange = millis();
    }
}