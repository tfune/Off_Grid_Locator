#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110x.h>

#include "display.h"

constexpr int DISPLAY_WIDTH = 128;
constexpr int DISPLAY_HEIGHT = 64;
constexpr int DISPLAY_ADDRESS = 0x3C;

static Adafruit_SH1106G display(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire);

bool displayInit() {
    if(!display.begin(DISPLAY_ADDRESS)) {
        return false;
    }

    display.clearDisplay();

    return true;
}

void displayStartup() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);

    display.setCursor(40, 4);
    display.print("OFF-GRID");

    display.setCursor(43, 14);
    display.print("LOCATOR");

    display.drawLine(10, 25, 117, 25, SH110X_WHITE);

    display.setCursor(46, 31);
    display.print("v0.1.0");

    display.setCursor(28, 45);
    display.print("Press Button");

    display.setCursor(31, 55);
    display.print("to Continue");

    display.display();
}