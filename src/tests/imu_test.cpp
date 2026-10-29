#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SH110X.h>
#include "imu.h"

static Adafruit_SH1106G oled(128, 64, &Wire);
static bool imuInitialized = false;
static bool displayInitialized = false;
static unsigned long lastPrint = 0;

void setup() {
    Wire.begin();

    displayInitialized = oled.begin(0x3C);
    imuInitialized = imuInit();

    if (displayInitialized) {
        oled.clearDisplay();
        oled.setTextSize(1);
        oled.setTextColor(SH110X_WHITE);
        oled.setCursor(0, 0);
        oled.println("IMU TEST");
        oled.println(imuInitialized ? "Waiting for readings" : "IMU init failed");
        oled.display();
    }
}

void loop() {
    if (imuInitialized) {
        imuUpdate();
    }

    if (displayInitialized && imuInitialized &&
        millis() - lastPrint >= 1000) {

        const float accuracyDegrees =
            imuGetAccuracy() * 180.0f / PI;

        oled.clearDisplay();

        oled.setCursor(0, 0);
        oled.print("IMU TEST");

        oled.setCursor(0, 18);
        oled.print("Heading: ");
        oled.print(imuGetHeading(), 1);
        oled.print(" deg");

        oled.setCursor(0, 34);
        oled.print("Est error: ");
        oled.print(accuracyDegrees, 2);
        oled.print(" deg");

        oled.setCursor(0, 50);
        oled.print("Required: <=5 deg");

        oled.display();
        lastPrint = millis();
    }
}