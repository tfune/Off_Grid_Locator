#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SH110X.h>
#include "input.h"

static Adafruit_SH1106G oled(128, 64, &Wire);
static bool displayReady = false;

static unsigned long clockwiseCount = 0;
static unsigned long counterclockwiseCount = 0;
static unsigned long buttonCount = 0;
static const char* lastEvent = "Waiting";

static void showInput() {
    if (!displayReady) return;

    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(SH110X_WHITE);

    oled.setCursor(0, 0);
    oled.print("ENCODER TEST");

    oled.setCursor(0, 14);
    oled.print(lastEvent);

    oled.setCursor(0, 28);
    oled.print("CW: ");
    oled.print(clockwiseCount);

    oled.setCursor(0, 40);
    oled.print("CCW: ");
    oled.print(counterclockwiseCount);

    oled.setCursor(0, 52);
    oled.print("Presses: ");
    oled.print(buttonCount);

    oled.display();
}

void setup() {
    Wire.begin();
    displayReady = oled.begin(0x3C);

    inputInit();
    showInput();
}

void loop() {
    inputUpdate();

    const int rotation = getRotation();
    const bool buttonPress = getButtonPress();
    bool changed = false;

    if (rotation > 0) {
        clockwiseCount += rotation;
        lastEvent = "Clockwise";
        changed = true;
    } else if (rotation < 0) {
        counterclockwiseCount += -rotation;
        lastEvent = "Counterclockwise";
        changed = true;
    }

    if (buttonPress) {
        buttonCount++;
        lastEvent = "Button pressed";
        changed = true;
    }

    // Limit screen refreshes while continuing to process input.
    static unsigned long lastRefresh = 0;
    static bool refreshPending = false;

    refreshPending |= changed;

    if (refreshPending && millis() - lastRefresh >= 100) {
        showInput();
        lastRefresh = millis();
        refreshPending = false;
    }
}