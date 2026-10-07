#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <Wire.h>
#include "display.h"
#include "input.h"
#include "battery.h"

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

    Wire.begin();

    batteryInit();
    displayInitialized = displayInit();
    inputInit();

    if(displayInitialized) {
        displayStartup();
    }
}

void loop() {
    static bool serialAnnounced = false;
    static unsigned long lastHeartbeat = 0;
    const unsigned long heartbeatNow = millis();

    if (Serial) {
        if (!serialAnnounced) {
            Serial.println("Battery test connected");
            serialAnnounced = true;
        }

        if (heartbeatNow - lastHeartbeat >= 1000) {
            lastHeartbeat = heartbeatNow;
            Serial.print("Battery test running, cached voltage: ");
            Serial.println(batteryGetVoltage(), 2);
        }
    } else {
        serialAnnounced = false;
    }
    
    static unsigned long lastBatteryDisplay = 0;

    if (batteryUpdate() && Serial) {
        Serial.print("Battery sampled at ");
        Serial.print(millis() / 1000);
        Serial.print(" s: ");
        Serial.println(batteryGetVoltage(), 2);
    }

    const unsigned long now = millis();

    if (displayInitialized && now - lastBatteryDisplay >= 60000) {
        lastBatteryDisplay = now;
        displayRefreshBattery();

        if (Serial) {
            Serial.println("Battery icon refreshed");
        }
    }
   
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
