#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include "device.h"
#include "imu.h"

constexpr uint16_t deviceAddress = 1; // Change this to 2 for the second device 

void setup() {
  deviceInit(deviceAddress);
}

void loop() {
  deviceUpdate();
}