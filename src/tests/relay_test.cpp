#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <math.h>

#include "device.h"
#include "gps.h"
#include "lora.h"
#include "packet.h"
#include "target.h"

static unsigned long lastLocationSend = 0;
static unsigned long locationSendInterval = 2000;
static unsigned int locationSequence = 0;

static unsigned int lastSequence[3] = {};
static unsigned long lastSequenceTime[3] = {};
static bool hasSequence[3] = {};

static bool relayPending = false;
static LocationPacket relayPacket;
static unsigned long relayTime = 0;

static bool isDuplicate(const LocationPacket& packet) {
    int index = packet.source - 1;
    unsigned long now = millis();

    constexpr unsigned long SEQUENCE_RESET_TIME = 10000;

    if(hasSequence[index] &&
       now - lastSequenceTime[index] < SEQUENCE_RESET_TIME &&
       packet.sequence <= lastSequence[index]) {
        return true;
    }

    lastSequence[index] = packet.sequence;
    lastSequenceTime[index] = now;
    hasSequence[index] = true;

    return false;
}

static void receiveLocations() {
    loraUpdate();

    LoRaMessage message;
    if(!loraReceive(message)) {
        return;
    }

    LocationPacket packet;
    if(!parseLocationPacket(message.payload, packet)) {
        return;
    }

    if(message.sender < 1 || message.sender > 3) {
        return;
    }

    if(packet.hops > 1 ||
       !isfinite(packet.latitude) ||
       !isfinite(packet.longitude)) {
        return;
    }

    if(packet.hops == 0 && packet.source != message.sender) {
        return;
    }

    if(packet.hops == 1 && packet.source == message.sender) {
        return;
    }

    if(packet.source == DEVICE_ID ||
       packet.source < 1 ||
       packet.source > 3) {
        return;
    }

    // Test condition:
    // Device 1 cannot receive Device 3 directly.
    if(DEVICE_ID == 1 &&
       message.sender == 3 &&
       packet.source == 3 &&
       packet.hops == 0) {

        Serial.println("Direct Device 3 packet blocked");
        return;
    }

    if(isDuplicate(packet)) {
        if(relayPending &&
           relayPacket.source == packet.source &&
           relayPacket.sequence == packet.sequence) {
            relayPending = false;
        }

        return;
    }

    Coordinates coordinates = {
        packet.latitude,
        packet.longitude
    };

    if(targetSet(packet.source, coordinates, millis())) {
        Serial.print("Location received from device ");
        Serial.print(packet.source);
        Serial.print(" via device ");
        Serial.print(message.sender);
        Serial.print(" | hops: ");
        Serial.println(packet.hops);

        if(DEVICE_ID == 1 &&
           packet.source == 3 &&
           message.sender == 2 &&
           packet.hops == 1) {
            Serial.println("RELAY TEST PASS");
        }
    }

    if(packet.hops == 0 && !relayPending) {
        relayPacket = packet;
        relayPacket.hops = 1;

        relayTime = millis() + random(100, 301);
        relayPending = true;
    }
}

static void sendRelay() {
    if(!loraIsInitialized() ||
       !relayPending ||
       millis() < relayTime) {
        return;
    }

    String payload = createLocationPacket(relayPacket);

    bool accepted = loraSend(0, payload);

    Serial.println(
        accepted ? "Location relay accepted"
                 : "Location relay failed"
    );

    relayPending = false;
}

static void sendLocation() {
    unsigned long now = millis();

    if(!loraIsInitialized() ||
       now - lastLocationSend < locationSendInterval) {
        return;
    }

    lastLocationSend = now;
    locationSendInterval =
        static_cast<unsigned long>(random(1800, 2201));

    GPSData gpsData = gpsGetData();

    if(!gpsHasFreshFix(now, 3000) ||
       !isfinite(gpsData.latitude) ||
       !isfinite(gpsData.longitude)) {
        return;
    }

    LocationPacket packet;
    packet.source = DEVICE_ID;
    packet.sequence = locationSequence++;
    packet.latitude = gpsData.latitude;
    packet.longitude = gpsData.longitude;
    packet.hops = 0;

    String payload = createLocationPacket(packet);

    bool accepted = loraSend(0, payload);

    Serial.println(
        accepted ? "Location broadcast accepted"
                 : "Location broadcast failed"
    );
}

void setup() {
    Serial.begin(115200);

    while(!Serial) {
        delay(10);
    }

    Serial.print("Relay test - Device ");
    Serial.println(DEVICE_ID);

    gpsInit();

    bool radioReady = loraInit(DEVICE_ID);

    Serial.println(
        radioReady ? "LoRa initialized"
                   : "LoRa initialization failed"
    );

    randomSeed(DEVICE_ID);

    lastLocationSend = millis();
    locationSendInterval =
        400UL + (DEVICE_ID - 1UL) * 600UL;
}

void loop() {
    gpsUpdate();

    receiveLocations();
    sendRelay();
    sendLocation();
}