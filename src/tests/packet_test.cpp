#include <Arduino.h>
#include "packet.h"

void setup() {
    Serial.begin(115200);

    while(!Serial) {
        delay(10);
    }

    LocationPacket packet = {
        1,
        15,
        32.715700,
        -117.161100,
        0
    };

    String data = createLocationPacket(packet);

    Serial.println("Created Packet:");
    Serial.println(data);

    LocationPacket receivedPacket;

    if(!parseLocationPacket(data, receivedPacket)) {
        Serial.println("PARSE FAILED");
        return;
    }

    Serial.println("Parsed Packet:");
    Serial.print("Source: ");
    Serial.println(receivedPacket.source);
    Serial.print("Sequence: ");
    Serial.println(receivedPacket.sequence);
    Serial.print("Latitude: ");
    Serial.println(receivedPacket.latitude, 6);
    Serial.print("Longitude: ");
    Serial.println(receivedPacket.longitude, 6);
    Serial.print("Hops: ");
    Serial.println(receivedPacket.hops);
}

void loop() {

}