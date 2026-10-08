#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <math.h>

#include "device.h"
#include "gps.h"
#include "input.h"
#include "lora.h"
#include "packet.h"

constexpr unsigned int DEVICE_COUNT =
    sizeof(devices) / sizeof(devices[0]);

static_assert(DEVICE_ID >= 1 && DEVICE_ID <= 3,
              "DEVICE_ID must be 1, 2, or 3");

struct ReceivedLocation {
    double latitude;
    double longitude;
    unsigned int sequence;
    unsigned long receivedAt;
    int rssi;
    int snr;
    bool available;
};

static ReceivedLocation received[DEVICE_COUNT] = {};
static Adafruit_SH1106G oled(128, 64, &Wire);

static bool displayReady = false;
static unsigned int selectedIndex = 0;
static unsigned int sequence = 0;

static unsigned long lastSend = 0;
static unsigned long sendInterval = 2000;
static unsigned long lastDisplay = 0;

static int findDevice(uint16_t id) {
    for (unsigned int i = 0; i < DEVICE_COUNT; ++i) {
        if (devices[i].id == id) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

static void selectMember(int direction) {
    do {
        if (direction > 0) {
            selectedIndex = (selectedIndex + 1) % DEVICE_COUNT;
        } else {
            selectedIndex =
                (selectedIndex + DEVICE_COUNT - 1) % DEVICE_COUNT;
        }
    } while (devices[selectedIndex].id == DEVICE_ID);
}

static void sendLocation() {
    unsigned long now = millis();

    if (!loraIsInitialized() ||
        now - lastSend < sendInterval) {
        return;
    }

    lastSend = now;
    sendInterval = static_cast<unsigned long>(random(1800, 2201));

    GPSData local = gpsGetData();

    if (!gpsHasFreshFix(now, 3000) ||
        !isfinite(local.latitude) ||
        !isfinite(local.longitude)) {
        return;
    }

    LocationPacket packet = {
        DEVICE_ID,
        sequence++,
        local.latitude,
        local.longitude,
        0
    };

    String payload = createLocationPacket(packet);
    bool accepted = loraSend(0, payload);

    if (Serial) {
        // Payload includes ID, sequence, and coordinates.
        Serial.print("TX ");
        Serial.print(payload);
        Serial.println(accepted ? " accepted" : " failed");
    }
}

static void receiveLocations() {
    loraUpdate();

    LoRaMessage message;
    while (loraReceive(message)) {
        LocationPacket packet;

        if (!parseLocationPacket(message.payload, packet)) {
            continue;
        }

        int index = findDevice(message.sender);

        if (index < 0 ||
            message.sender == DEVICE_ID ||
            packet.source != message.sender ||
            packet.hops != 0 ||
            !isfinite(packet.latitude) ||
            !isfinite(packet.longitude)) {
            continue;
        }

        ReceivedLocation& location = received[index];
        location.latitude = packet.latitude;
        location.longitude = packet.longitude;
        location.sequence = packet.sequence;
        location.receivedAt = millis();
        location.rssi = message.rssi;
        location.snr = message.snr;
        location.available = true;

        if (Serial) {
            Serial.print("RX ");
            Serial.print(message.payload);
            Serial.print(" RSSI=");
            Serial.print(message.rssi);
            Serial.print(" SNR=");
            Serial.println(message.snr);

            GPSData local = gpsGetData();
            Serial.print("LOCAL id=");
            Serial.print(DEVICE_ID);
            Serial.print(" fresh=");
            Serial.print(gpsHasFreshFix(millis(), 3000));
            Serial.print(" lat=");
            Serial.print(local.latitude, 6);
            Serial.print(" lon=");
            Serial.println(local.longitude, 6);
        }
    }
}

static void updateDisplay() {
    unsigned long now = millis();

    if (!displayReady || now - lastDisplay < 250) {
        return;
    }

    lastDisplay = now;
    oled.clearDisplay();
    oled.setCursor(0, 0);

    if (!loraIsInitialized()) {
        oled.println("LoRa init failed");
        oled.println("Reset to retry");
        oled.display();
        return;
    }

    oled.print("RX: ");
    oled.println(devices[selectedIndex].name);

    const ReceivedLocation& location = received[selectedIndex];

    if (!location.available) {
        oled.println("No update received");
    } else {
        unsigned long age = now - location.receivedAt;

        oled.print("Lat:");
        oled.println(location.latitude, 6);
        oled.print("Lon:");
        oled.println(location.longitude, 6);

        oled.print("Seq:");
        oled.print(location.sequence);
        oled.print(" Age:");
        oled.print(age / 1000);
        oled.println("s");

        oled.print("RSSI:");
        oled.print(location.rssi);
        oled.print(" SNR:");
        oled.println(location.snr);

        if (age > 30000) {
            oled.println("STALE");
        } else {
            oled.println(gpsHasFreshFix(now, 3000)
                             ? "Local GPS: ready"
                             : "Local GPS: waiting");
        }
    }

    oled.display();
}

void setup() {
    Serial.begin(115200);
    Wire.begin();

    gpsInit();
    inputInit();

    displayReady = oled.begin(0x3C);
    if (displayReady) {
        oled.setTextSize(1);
        oled.setTextColor(SH110X_WHITE);
        oled.setTextWrap(false);
    }

    while (devices[selectedIndex].id == DEVICE_ID) {
        selectedIndex = (selectedIndex + 1) % DEVICE_COUNT;
    }

    loraInit(DEVICE_ID);

    randomSeed(DEVICE_ID);
    lastSend = millis();
    sendInterval = 400UL + (DEVICE_ID - 1UL) * 600UL;
}

void loop() {
    gpsUpdate();
    receiveLocations();
    inputUpdate();

    int rotation = getRotation();
    if (rotation != 0) {
        selectMember(rotation > 0 ? 1 : -1);
    }

    // Consume button events; this test uses rotation only.
    getButtonPress();

    sendLocation();
    updateDisplay();
}