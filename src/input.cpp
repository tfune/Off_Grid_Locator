#include <Arduino.h>
#include "input.h"

constexpr int ENCODER_SW = A0;
constexpr int ENCODER_DT = A1;
constexpr int ENCODER_CLK = A2;

constexpr unsigned long DEBOUNCE_TIME = 50;

static volatile int previousEncoderState = 0;
static volatile int encoderMovement = 0;

static int rotation = 0;

static bool previousButtonState = HIGH;
static bool buttonPress = false;
static unsigned long lastButtonPress = 0;

static const int transitionTable[16] {
    0, -1, 1, 0,
    1, 0, 0, -1,
    -1, 0, 0, 1,
    0, 1, -1, 0
};

static int getEncoderState() {
    int clk = digitalRead(ENCODER_CLK);
    int dt = digitalRead(ENCODER_DT);

    return (clk << 1) | dt;
}

static void encoderISR() {
    int currentEncoderState = getEncoderState();

    int index = (previousEncoderState << 2) | currentEncoderState;
    encoderMovement += transitionTable[index];

    previousEncoderState = currentEncoderState;
}

void inputInit() {
    pinMode(ENCODER_CLK, INPUT_PULLUP);
    pinMode(ENCODER_DT, INPUT_PULLUP);
    pinMode(ENCODER_SW, INPUT_PULLUP);

    previousEncoderState = getEncoderState();
    previousButtonState = digitalRead(ENCODER_SW);

    attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), encoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODER_DT), encoderISR, CHANGE);
}

void inputUpdate() {
    if(encoderMovement >= 4) {
        rotation++;
        encoderMovement -= 4;
    }
    else if(encoderMovement <= -4) {
        rotation--;
        encoderMovement += 4;
    }

    bool buttonState = digitalRead(ENCODER_SW);

    if(buttonState == LOW &&
        previousButtonState == HIGH &&
        millis() - lastButtonPress >= DEBOUNCE_TIME) {

        buttonPress = true;
        lastButtonPress = millis();
    }

    previousButtonState = buttonState;
}

int getRotation() {
    int value = rotation;
    rotation = 0;

    return value;
}

bool getButtonPress() {
    if(buttonPress) {
        buttonPress = false;
        return true;
    }

    return false;
}