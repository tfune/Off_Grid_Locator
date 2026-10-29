#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SH110X.h>
#include "gps.h"

static Adafruit_SH1106G oled(128, 64, &Wire);
static bool displayReady = false;

static void showGPS() {
    const GPSData data = gpsGetData();
    const bool communicating = gpsIsCommunicating();
    const bool freshFix = gpsHasFreshFix(millis(), 3000);

    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(SH110X_WHITE);

    oled.setCursor(0, 0);
    oled.print("GPS TEST");

    oled.setCursor(0, 12);
    oled.print("Data: ");
    oled.print(communicating ? "Receiving" : "No data");

    oled.setCursor(0, 22);
    oled.print("Fix: ");
    oled.print(freshFix ? "Valid" : "Unavailable");

    oled.setCursor(0, 32);
    oled.print("Lat: ");
    if (freshFix) {
        oled.print(data.latitude, 6);
    } else {
        oled.print("--");
    }

    oled.setCursor(0, 42);
    oled.print("Lon: ");
    if (freshFix) {
        oled.print(data.longitude, 6);
    } else {
        oled.print("--");
    }

    oled.setCursor(0, 54);
    oled.print("UTC: ");
    if (freshFix) {
        char timeText[9];
        snprintf(timeText, sizeof(timeText), "%02d:%02d:%02d",
                 data.time / 10000,
                 (data.time / 100) % 100,
                 data.time % 100);
        oled.print(timeText);
    } else {
        oled.print("--:--:--");
    }

    oled.display();
}

void setup() {
    Wire.begin();
    displayReady = oled.begin(0x3C);

    if (displayReady) {
        oled.clearDisplay();
        oled.setTextSize(1);
        oled.setTextColor(SH110X_WHITE);
        oled.setCursor(0, 0);
        oled.print("GPS initializing...");
        oled.display();
    }

    gpsInit();
    if (displayReady) {
        showGPS();
    }
}

void loop() {
    gpsUpdate();

    static unsigned long lastRefresh = 0;
    const unsigned long now = millis();

    if (displayReady && now - lastRefresh >= 1000UL) {
        lastRefresh = now;
        showGPS();
    }
}