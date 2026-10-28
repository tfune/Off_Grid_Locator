#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SH110X.h>
#include "battery.h"

static Adafruit_SH1106G oled(128, 64, &Wire);
static bool displayReady = false;

static void showBattery() {
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(SH110X_WHITE);

    oled.setCursor(0, 0);
    oled.print("BATTERY TEST");
    oled.drawLine(0, 10, 127, 10, SH110X_WHITE);

    oled.setCursor(0, 18);
    oled.print("Voltage: ");
    oled.print(batteryGetVoltage(), 2);
    oled.print(" V");

    oled.setCursor(0, 30);
    oled.print("Level: ");
    oled.print(batteryGetLevel());
    oled.print("/4");

    oled.setCursor(0, 42);
    oled.print("Uptime: ");
    oled.print(millis() / 1000UL);
    oled.print(" s");

    oled.setCursor(0, 54);
    oled.print("Sample period: 30 s");

    // Transfer the drawing buffer to the physical OLED.
    oled.display();
}

void setup() {
    Wire.begin();
    batteryInit();

    displayReady = oled.begin(0x3C);

    if (displayReady) {
        showBattery();
    }
}

void loop() {
    // Continue sampling even if display initialization failed.
    const bool newSample = batteryUpdate();

    static unsigned long lastRefresh = 0;
    const unsigned long now = millis();

    if (displayReady &&
        (newSample || now - lastRefresh >= 1000UL)) {
        lastRefresh = now;
        showBattery();
    }
}