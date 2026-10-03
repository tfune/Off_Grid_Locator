#include <Arduino.h>
#include "gps.h"

static unsigned long lastPrint = 0;

void setup() {
    Serial.begin(115200);

    while(!Serial) {
        delay(10);
    }

    Serial.println("GPS test started");

    gpsInit();
}

void loop() {
    gpsUpdate();

    if(millis() - lastPrint >= 1000) {
        GPSData gpsData = gpsGetData();

        Serial.print("GPS Fix: ");
        Serial.println(gpsData.fix ? "Yes" : "No");

        Serial.print("Latitude: ");
        Serial.println(gpsData.latitude, 6);

        Serial.print("Longitude: ");
        Serial.println(gpsData.longitude, 6);

        Serial.print("Time: ");
        Serial.println(gpsData.time);

        Serial.println("-------------------------");

        lastPrint = millis();
    }
}