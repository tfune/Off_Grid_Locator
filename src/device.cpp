#include <Arduino.h>
#include <Wire.h>
#include <math.h>

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

// Battery display refresh timing
static unsigned long lastBatteryDisplay = 0;

// Track initialization of components required for the UI
static bool displayInitialized = false;
static bool imuInitialized = false;

// User interface states
enum Screen {
    STARTUP,
    MEMBER_LIST,
    TRACKING
};

static Screen currentScreen = STARTUP;

// Group members available for selection and tracking
static Device firstDevice;
static Device secondDevice;
static Device selectedDevice;

// Location broadcast timing and packet sequence
static unsigned long lastLocationSend = 0;
static unsigned long locationSendInterval = 0;
static unsigned int locationSequence = 0;

// Track the latest packet from each device for duplicate detection
static unsigned int lastSequence[3] = {};
static unsigned long lastSequenceTime[3] = {};
static bool hasSequence[3] = {};

// Pending one-hop relay state
static bool relayPending = false;
static LocationPacket relayPacket;
static unsigned long relayTime = 0;

void deviceInit() {
    // Populate the member list with the two devices other than this device
    if (DEVICE_ID == 1) {
        firstDevice = devices[1];
        secondDevice = devices[2];
    }
    else if (DEVICE_ID == 2) {
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

    if (displayInitialized) {
        displayStartup();
    }

    loraInit(DEVICE_ID);

    randomSeed(DEVICE_ID);

    // Stagger each device's first broadcast to reduce startup collisions
    lastLocationSend = millis();
    locationSendInterval = 400UL + (DEVICE_ID - 1UL) * 600UL;
}

static bool isDuplicate(const LocationPacket& packet) {
    // Device IDs 1-3 map to array indexes 0-2
    int index = packet.source - 1;
    unsigned long now = millis();

    // Allow sequence numbers to restart after a device has been silent
    constexpr unsigned long SEQUENCE_RESET_TIME_MS = 10000;

    if (hasSequence[index] && 
        now - lastSequenceTime[index] < SEQUENCE_RESET_TIME_MS &&
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
    if (!loraReceive(message)) {
        return;
    }

    LocationPacket packet;
    if (!parseLocationPacket(message.payload, packet)) {
        return;
    }

    // Reject packets from invalid senders
    if (message.sender < 1 || message.sender > 3) {
        return;
    }

    // Only direct and one-hop packets with valid coordinates are supported
    if (packet.hops > 1 || !isfinite(packet.latitude) || !isfinite(packet.longitude)) {
        return;
    }

    // Direct packets must originate from the transmitting device
    if (packet.hops == 0 && packet.source != message.sender) {
        return;
    }

    // Relayed packets must originate from a different device than the relay
    if (packet.hops == 1 && packet.source == message.sender) {
        return;
    }

    // Ignore this device's own location and invalid source IDs
    if (packet.source == DEVICE_ID || packet.source < 1 || packet.source > 3) {
        return;
    }

    // Ignore packets that have already been processed
    if (isDuplicate(packet)) {
        // Cancel a pending relay if another device already relayed this packet
        if (relayPending && 
            relayPacket.source == packet.source && 
            relayPacket.sequence == packet.sequence) {
            relayPending = false;
        }

        return;
    }

    Coordinates coordinates = {packet.latitude, packet.longitude};

    if (targetSet(packet.source, coordinates, millis())) {
        Serial.print("Location received from device ");
        Serial.print(packet.source);
        Serial.print(" via device ");
        Serial.print(message.sender);
        Serial.print(" | hops: ");
        Serial.println(packet.hops);
    }

    // Only directly received packets are eligible for one-hop relay
    if (packet.hops == 0 && !relayPending) {
        relayPacket = packet;
        relayPacket.hops = 1;

        // Random delay reduces collisions when multiple devices attempt to relay
        relayTime = millis() + random(100, 301);
        relayPending = true;
    }
}

static void sendRelay() {
    // Wait until a relay is pending and its randomized delay has expired
    if (!loraIsInitialized() || !relayPending || millis() < relayTime) {
        return;
    }

    String payload = createLocationPacket(relayPacket);
    bool accepted = loraSend(0, payload);

    Serial.println(accepted ? "Location relay accepted" : "Location relay failed");

    relayPending = false;
}

static void sendLocation() {
    unsigned long now = millis();

    if (!loraIsInitialized() || now - lastLocationSend < locationSendInterval) {
        return;
    }

    lastLocationSend = now;

    // Randomize recurring broadcasts to reduce repeated transmission collisions
    locationSendInterval = static_cast<unsigned long>(random(1800, 2201));

    GPSData gpsData = gpsGetData();

    // Only broadcast a recent and valid GPS position
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

    Serial.println(accepted ? "Location broadcast accepted" : "Location broadcast failed");
}

void deviceUpdate() {
    // Update background hardware and location-sharing tasks
    batteryUpdate();
    gpsUpdate();
    receiveLocations();
    sendRelay();
    sendLocation();

    const unsigned long now = millis();

    // Refresh the battery indicator once per minute
    if (displayInitialized && now - lastBatteryDisplay >= 60000) {
        lastBatteryDisplay = now;
        displayRefreshBattery();
    }

    // Process user input and update the active UI screen
    if (displayInitialized && imuInitialized) {
        imuUpdate();
        inputUpdate();

        int rotation = getRotation();
        bool buttonPress = getButtonPress();

        switch (currentScreen) {
            case STARTUP:
                if (buttonPress) {
                    currentScreen = MEMBER_LIST;
                    displayMemberList(firstDevice, secondDevice, selectedDevice);
                }
                break;

            case MEMBER_LIST:
                if (rotation > 0) {
                    selectedDevice = secondDevice;
                    displayMemberList(firstDevice, secondDevice, selectedDevice);
                }
                else if (rotation < 0) {
                    selectedDevice = firstDevice;
                    displayMemberList(firstDevice, secondDevice, selectedDevice);
                }

                if (buttonPress) {
                    currentScreen = TRACKING;
                }
                break;

            case TRACKING: {
                if (buttonPress) {
                    currentScreen = MEMBER_LIST;
                    displayMemberList(firstDevice, secondDevice, selectedDevice);
                    break;
                }

                GPSData gpsData = gpsGetData();

                // Tracking requires a recent GPS fix from this device
                if (!gpsHasFreshFix(now, 3000)) {
                    displayLocationUnavailable();
                    break;
                }
                
                Coordinates currentCoordinates = {gpsData.latitude, gpsData.longitude};
                Coordinates targetCoordinates;

                // Received locations older than 30 secs are no longer valid for tracking
                constexpr unsigned long LOCATION_MAX_AGE_MS = 30000;

                if (!targetGet(selectedDevice.id, targetCoordinates, now, LOCATION_MAX_AGE_MS)) {
                    displayLocationUnavailable();
                    break;
                }

                // Calculate the selected member's distance and bearing
                NavigationResult result = calculateNavigation(currentCoordinates, targetCoordinates);

                constexpr double proxDistance = 25;
                bool withinProximity = result.distance <= proxDistance;

                if (!result.bearingValid && !withinProximity) {
                    displayLocationUnavailable();
                    break;
                }

                // Convert absolute bearing into arrow angle relative to device heading
                float heading = imuGetHeading();
                float arrowAngle = calculateArrowAngle(result.bearing, heading);

                constexpr float imuWarningRad = 5 * PI / 180;

                float headingAccuracy = imuGetAccuracy();
                bool headingWarning = !isfinite(headingAccuracy) || headingAccuracy > imuWarningRad;

                displayTracking(selectedDevice, result.distance, result.bearing, arrowAngle, headingWarning, withinProximity);

                break;
            }
        }
    }
}