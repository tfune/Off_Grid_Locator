#include <Arduino.h>
#include <Wire.h>
#include "device.h"
#include "imu.h"
#include "gps.h"
#include "display.h"
#include "input.h"
#include "lora.h"
#include "navigation.h"
#include "battery.h"
#include "target.h"
#include "packet.h"
#include <math.h>

static unsigned long lastBatteryDisplay = 0;

static bool displayInitialized = false;
static bool imuInitialized = false;

enum Screen {
    STARTUP,
    MEMBER_LIST,
    TRACKING
};

static Screen currentScreen = STARTUP;

static Device firstDevice;
static Device secondDevice;
static Device selectedDevice;

static unsigned long lastLocationSend = 0;
static unsigned long locationSendInterval = 2000;
static unsigned int locationSequence = 0;

static unsigned int lastSequence[3] = {};
static unsigned long lastSequenceTime[3] = {};
static bool hasSequence[3] = {};

static bool relayPending = false;
static LocationPacket relayPacket;
static unsigned long relayTime = 0;

void setup() {
    if(DEVICE_ID == 1) {
        firstDevice = devices[1];
        secondDevice = devices[2];
    }
    else if(DEVICE_ID == 2) {
        firstDevice = devices[0];
        secondDevice = devices[2];
    }
    else {
        firstDevice = devices[0];
        secondDevice = devices[1];
    }

    selectedDevice = firstDevice;

    Serial.begin(115200);
    Wire.begin();

    batteryInit();
    lastBatteryDisplay = millis();

    displayInitialized = displayInit();
    gpsInit();
    imuInitialized = imuInit();
    inputInit();

    if(displayInitialized) {
        displayStartup();
    }

    loraInit(DEVICE_ID);

    randomSeed(DEVICE_ID);

    lastLocationSend = millis();
    locationSendInterval = 400UL + (DEVICE_ID - 1UL) * 600UL;
}

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
    if (!loraReceive(message)){
        return;
    }

    LocationPacket packet;
    if (!parseLocationPacket(message.payload, packet)) {
        return;
    }

    if (message.sender < 1 || message.sender > 3) {
        return;
    }

    if (packet.hops > 1 ||
        !isfinite(packet.latitude) ||
        !isfinite(packet.longitude)) {
        return;
    }

    if (packet.hops == 0 && packet.source != message.sender) {
        return;
    }

    if (packet.hops == 1 && packet.source == message.sender) {
        return;
    }

    if (packet.source == DEVICE_ID ||
        packet.source < 1 ||
        packet.source > 3) {
        return;
    }

    // TEST ONLY:
    // Device 1 cannot receive Device 3 directly.
    if(DEVICE_ID == 1 &&
       message.sender == 3 &&
       packet.source == 3 &&
       packet.hops == 0) {
        Serial.println("TEST: Direct Device 3 packet blocked");
        return;
    }

    if (isDuplicate(packet)) {
        if(relayPending &&
           relayPacket.source == packet.source &&
           relayPacket.sequence == packet.sequence) {
            relayPending = false;
        }

        return;
    }

    Coordinates coordinates = {
        packet.latitude, packet.longitude
    };

    if (targetSet(packet.source, coordinates, millis())) {
        Serial.print("Location received from device ");
        Serial.print(packet.source);
        Serial.print(" via device ");
        Serial.print(message.sender);
        Serial.print(" | hops: ");
        Serial.println(packet.hops);

        // TEST ONLY:
        // Device 1 successfully received Device 3 through Device 2.
        if(DEVICE_ID == 1 &&
           packet.source == 3 &&
           message.sender == 2 &&
           packet.hops == 1) {
            Serial.println("TEST: RELAY PASS");
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

    if (!loraIsInitialized() ||
        now - lastLocationSend < locationSendInterval) {
        return;
    }

    lastLocationSend = now;
    locationSendInterval =
        static_cast<unsigned long>(random(1800, 2201));

    GPSData gpsData = gpsGetData();

    if (!gpsHasFreshFix(now, 3000) ||
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

void loop() {
    batteryUpdate();
    gpsUpdate();
    receiveLocations();
    sendRelay();
    sendLocation();

    const unsigned long now = millis();

    if (displayInitialized &&
        now - lastBatteryDisplay >= 60000) {
        lastBatteryDisplay = now;
        displayRefreshBattery();
    }

    if(displayInitialized && imuInitialized) {
        imuUpdate();
        inputUpdate();

        int rotation = getRotation();
        bool buttonPress = getButtonPress();

        switch(currentScreen) {
            case STARTUP:
                if(buttonPress) {
                    currentScreen = MEMBER_LIST;
                    displayMemberList(
                        firstDevice,
                        secondDevice,
                        selectedDevice
                    );
                }
                break;

            case MEMBER_LIST:
                if(rotation > 0) {
                    selectedDevice = secondDevice;
                    displayMemberList(
                        firstDevice,
                        secondDevice,
                        selectedDevice
                    );
                }
                else if(rotation < 0) {
                    selectedDevice = firstDevice;
                    displayMemberList(
                        firstDevice,
                        secondDevice,
                        selectedDevice
                    );
                }

                if(buttonPress) {
                    currentScreen = TRACKING;
                }
                break;

            case TRACKING: {
                if(buttonPress) {
                    currentScreen = MEMBER_LIST;
                    displayMemberList(
                        firstDevice,
                        secondDevice,
                        selectedDevice
                    );
                    break;
                }

                GPSData gpsData = gpsGetData();

                if(!gpsHasFreshFix(millis(), 3000)) {
                    displayLocationUnavailable();
                    break;
                }

                Coordinates currentCoordinates = {
                    gpsData.latitude,
                    gpsData.longitude
                };

                Coordinates targetCoordinates;

                constexpr unsigned long LOCATION_MAX_AGE_MS = 30000;

                if (!targetGet(
                        selectedDevice.id,
                        targetCoordinates,
                        millis(),
                        LOCATION_MAX_AGE_MS)) {
                    displayLocationUnavailable();
                    break;
                }

                NavigationResult result =
                    calculateNavigation(
                        currentCoordinates,
                        targetCoordinates
                    );

                if(!result.bearingValid) {
                    displayLocationUnavailable();
                    break;
                }

                float heading = imuGetHeading();

                float arrowAngle =
                    calculateArrowAngle(
                        result.bearing,
                        heading
                    );

                displayTracking(
                    selectedDevice,
                    result.distance,
                    result.bearing,
                    arrowAngle
                );

                break;
            }
        }
    }
}