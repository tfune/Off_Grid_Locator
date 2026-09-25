#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110x.h>

#include "display.h"

constexpr int DISPLAY_WIDTH = 128;
constexpr int DISPLAY_HEIGHT = 64;
constexpr int DISPLAY_ADDRESS = 0x3C;

static Adafruit_SH1106G display(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire);

static void drawDirectionArrow(float angle) {
    const int centerX = 96;
    const int centerY = 31;
    const int length = 16;
    const int headLength = 5;
    const float headAngle = 30.0;

    float radians = angle * PI / 180.0;

    int endX = centerX + round(length * sin(radians));
    int endY = centerY - round(length * cos(radians));

    display.drawLine(centerX, centerY, endX, endY, SH110X_WHITE);

    float leftRadians = (angle + 180.0 - headAngle) * PI / 180.0;
    float rightRadians = (angle + 180.0 + headAngle) * PI / 180.0;

    int leftX = endX + round(headLength * sin(leftRadians));
    int leftY = endY - round(headLength * cos(leftRadians));

    int rightX = endX + round(headLength * sin(rightRadians));
    int rightY = endY - round(headLength * cos(rightRadians));

    display.drawLine(endX, endY, leftX, leftY, SH110X_WHITE);
    display.drawLine(endX, endY, rightX, rightY, SH110X_WHITE);
}

static const char* getCardinalDirection(float angle) {
    if(angle >= 337.5 || angle < 22.5) {
        return "N";
    }
    else if(angle < 67.5) {
        return "NE";
    }
    else if(angle < 112.5) {
        return "E";
    }
    else if(angle < 157.5) {
        return "SE";
    }
    else if(angle < 202.5) {
        return "S";
    }
    else if(angle < 247.5) {
        return "SE";
    }
    else if(angle < 292.5) {
        return "W";
    }
    else {
        return "NW";
    }
}

bool displayInit() {
    if(!display.begin(DISPLAY_ADDRESS)) {
        return false;
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);

    return true;
}

void displayStartup() {
    display.clearDisplay();

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

void displayMemberList() {
    display.clearDisplay();

    display.setCursor(0, 0);
    display.print("SELECT MEMBER");

    display.drawLine(0, 10, 127, 10, SH110X_WHITE);

    display.setCursor(0, 16);
    display.print("> Sebastian");

    display.setCursor(0, 28);
    display.print("  Professor Salemi");

    display.display();
}

void displayTracking(float distance, float direction) {
    display.clearDisplay();

    display.setCursor(0, 0);
    display.print("SEBASTIAN");

    display.drawLine(0, 10, 127, 10, SH110X_WHITE);

    display.setCursor(0, 15);
    display.print("Distance:");

    display.setCursor(0, 25);
    display.print(distance, 0);
    display.print(" m");

    display.setCursor(0, 39);
    display.print("Direction:");

    display.setCursor(0, 49);
    display.print(direction, 0);
    display.print(" deg");

    drawDirectionArrow(direction);

    display.setCursor(92, 52);
    display.print(getCardinalDirection(direction));

    display.display();
}

void displayLocationUnavailable() {
    display.clearDisplay();

    display.setCursor(0, 0);
    display.print("ERROR");

    display.drawLine(0, 10, 127, 10, SH110X_WHITE);

    display.setCursor(0, 18);
    display.print("Location unavailable");

    display.setCursor(0, 38);
    display.print("Waiting for update...");

    display.display();
}

void displayInitializationError() {
    display.clearDisplay();

    display.setCursor(0, 0);
    display.print("ERROR");

    display.drawLine(0, 10, 127, 10, SH110X_WHITE);

    display.setCursor(0, 18);
    display.print("Initialization failed");

    display.setCursor(0, 38);
    display.print("Reset device to retry");

    display.display();
}