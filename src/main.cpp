#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include "device.h"



#include "imu.h"

void setup() {
  Device_Init();
}

void loop() {
  Device_Update();
}