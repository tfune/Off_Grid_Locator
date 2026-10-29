#include <Arduino.h>
#include <Wire.h>
#include "display.h"
#include "device.h"

static bool displayInitialized = false;
static unsigned long lastScreenChange = 0;
static int currentScreen = 0;

static const Device firstDevice = {2, "Trevor"};
static const Device secondDevice = {3, "Professor Salemi"};
static const Device selectedDevice = firstDevice;

void setup() {
    Serial.begin(115200);

    while (!Serial) {
        delay(10);
    }

    Serial.println("Display test started");

    Wire.begin();

    displayInitialized = displayInit();

    if (!displayInitialized) {
        Serial.println("Display initialization failed");
        return;
    }

    displayStartup();
}

void loop() {
    if (!displayInitialized) {
        return;
    }

    // Cycle through each display screen every three seconds
    if (millis() - lastScreenChange >= 3000) {
        currentScreen++;

        if (currentScreen >4) {
            currentScreen = 0;
        }

        switch (currentScreen) {
            case 0:
                displayStartup();
                break;

            case 1:
                displayMemberList(firstDevice, secondDevice, selectedDevice);
                break;

            case 2:
                displayTracking(selectedDevice, 125.0, 225.0, 225.0);
                break;

            case 3:
                displayLocationUnavailable();
                break;

            case 4:
                displayInitializationError("IMU");
                break;
        }

        lastScreenChange = millis();
    }
}