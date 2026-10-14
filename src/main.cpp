#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include "device.h"

void setup() {
  deviceInit();
}

void loop() {
  deviceUpdate();
}