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

    while(!Serial) {
        delay(10);
    }

    Serial.println("Battery test started");

    Wire.begin();

    displayInitialized = displayInit();
    inputInit();

    if(displayInitialized) {
        displayStartup();
    }
}

void loop() {
    
    static unsigned long lastBatteryRead = 0;
    unsigned long now = millis();

    if (now - lastBatteryRead >= 1000) {
        lastBatteryRead = now;

        Serial.println("Reading battery...");
        float voltage = batteryReadVoltage();

        Serial.print("VBat: ");
        Serial.println(voltage, 2);
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