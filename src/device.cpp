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

//static const Coordinates sebastianTarget = {36.20625, -86.28833};
//static const Coordinates trevorTarget = {32.65272, -117.08230};
//static const Coordinates professorSalemiTarget = {34.05224, -118.24368};

static Device firstDevice;
static Device secondDevice;
static Device selectedDevice;

static unsigned long lastLocationSend = 0;
static unsigned long locationSendInterval = 2000;
static unsigned int locationSequence = 0;

void deviceInit() {
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

    if (packet.source != message.sender || packet.hops != 0 || !isfinite(packet.latitude) || !isfinite(packet.longitude)) {
        return;
    }

    Coordinates coordinates = {
        packet.latitude, packet.longitude
    };

    if (targetSet(message.sender, coordinates, millis())) {
        Serial.print("Location received from device ");
        Serial.println(message.sender);
    }
}

static void sendLocation() {
    unsigned long now = millis();

    if (!loraIsInitialized() || now - lastLocationSend < locationSendInterval) {
        return;
    }

    lastLocationSend = now;
    locationSendInterval =
    static_cast<unsigned long>(random(1800, 2201));

    GPSData gpsData = gpsGetData();

    if (!gpsHasFreshFix(now, 3000) || !isfinite(gpsData.latitude) || !isfinite(gpsData.longitude)) {
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

    Serial.println(accepted ? "Location broadcast accepted" : "Location broadcast failed");
}

void deviceUpdate() {
   
    batteryUpdate();
    gpsUpdate();
    receiveLocations();
    sendLocation();

    const unsigned long now = millis();

    if (displayInitialized && now - lastBatteryDisplay >= 60000) {
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
                    displayMemberList(firstDevice, secondDevice, selectedDevice);
                }
                break;

            case MEMBER_LIST:
                if(rotation > 0) {
                    selectedDevice = secondDevice;
                    displayMemberList(firstDevice, secondDevice, selectedDevice);
                }
                else if(rotation < 0) {
                    selectedDevice = firstDevice;
                    displayMemberList(firstDevice, secondDevice, selectedDevice);
                }

                if(buttonPress) {
                    currentScreen = TRACKING;
                }
                break;

            case TRACKING: {
                if(buttonPress) {
                    currentScreen = MEMBER_LIST;
                    displayMemberList(firstDevice, secondDevice, selectedDevice);
                    break;
                }

                GPSData gpsData = gpsGetData();

                if(!gpsHasFreshFix(millis(), 3000)) {
                    displayLocationUnavailable();
                    break;
                }
                
                Coordinates currentCoordinates = {gpsData.latitude, gpsData.longitude};
                Coordinates targetCoordinates;

                constexpr unsigned long LOCATION_MAX_AGE_MS = 30000;

                if (!targetGet(selectedDevice.id, targetCoordinates, millis(), LOCATION_MAX_AGE_MS)) {
                    displayLocationUnavailable();
                    break;
                }

                NavigationResult result = calculateNavigation(currentCoordinates, targetCoordinates);

                if(!result.bearingValid) {
                    displayLocationUnavailable();
                    break;
                }

                float heading = imuGetHeading();
                float arrowAngle = calculateArrowAngle(result.bearing, heading);

                displayTracking(selectedDevice, result.distance, result.bearing, arrowAngle);

                break;
            }
        }
    }
}