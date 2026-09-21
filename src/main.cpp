#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <Wire.h>

#include "imu.h"

void setup() {
  Serial.begin(115200);
  Wire.begin();

  if(IMU_Init() == false) {
    Serial.println("IMU Initialization Failed");
    return;
  }

  Serial.println("IMU Initialization Successful");
}

void loop() {
  IMU_Update();
}