#include <Arduino.h>
#include "packet.h"

void setup() {
    Serial.begin(115200);

    while (!Serial) {
        delay(10);
    }

    // Create test packet
    LocationPacket packet = {1, 15, 32.715700, -117.161100, 0};

    // Test createLocationPacket()
    String data = createLocationPacket(packet);
    Serial.println("Created packet:");
    Serial.println(data);

    // Test parseLocationPacket()
    LocationPacket parsedPacket;

    if (parseLocationPacket(data, parsedPacket)) {
        Serial.println("\nParsed packet:");
        Serial.print("Source: ");
        Serial.println(parsedPacket.source);
        Serial.print("Sequence: ");
        Serial.println(parsedPacket.sequence);
        Serial.print("Latitude: ");
        Serial.println(parsedPacket.latitude, 6);
        Serial.print("Longitude: ");
        Serial.println(parsedPacket.longitude, 6);
        Serial.print("Hops: ");
        Serial.println(parsedPacket.hops);
        Serial.println("Valid packet PASS");
    } else {
        Serial.println("Valid packet FAIL");
    }

    // Test invalid data inside packet
    Serial.println("\nTesting invalid packets:");

    if (!parseLocationPacket("LOC,1,15,91.0,-117.0,0", parsedPacket)) {
        Serial.println("Invalid latitude PASS");
    } else {
        Serial.println("Invalid latitude FAIL");
    }

    if (!parseLocationPacket("LOC,1,15,32.0,-181.0,0", parsedPacket)) {
        Serial.println("Invalid longitude PASS");
    } else {
        Serial.println("Invalid longitude FAIL");
    }

    if (!parseLocationPacket("LOC,1,15,abc,-117.0,0", parsedPacket)) {
        Serial.println("Invalid data PASS");
    } else {
        Serial.println("Invalid data FAIL");
    }
}

void loop() {
}