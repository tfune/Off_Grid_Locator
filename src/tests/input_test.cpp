#include <Arduino.h>
#include "input.h"

void setup() {
    Serial.begin(115200);

    while(!Serial) {
        delay(10);
    }

    Serial.println("Input test started");

    inputInit();
}

void loop() {
    inputUpdate();

    int rotation = getRotation();

    if(rotation > 0) {
        Serial.println("Clockwise");
    }
    else if(rotation < 0) {
        Serial.println("Counterclockwise");
    }

    if(getButtonPress()) {
        Serial.println("Button pressed");
    }
}