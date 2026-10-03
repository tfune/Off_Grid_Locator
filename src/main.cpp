#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include "device.h"



#include "imu.h"

void setup() {
  deviceInit(2);
}

void loop() {
  deviceUpdate();
}