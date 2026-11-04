#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <Wire.h>
#include <Adafruit_SH110X.h>
#include <math.h>
#include "device.h"
#include "gps.h"
#include "lora.h"
#include "packet.h"

constexpr unsigned long TEST_MS = 120000;
static Adafruit_SH1106G oled(128, 64, &Wire);
static bool ready, running, done;
static unsigned long startTime, lastCheck, lastTx, lastAttempt, lastDisplay;
static unsigned long checkMax, txMax, interval;
static unsigned int sequence, sent;

static void show(unsigned long now) {
    oled.clearDisplay();
    oled.setCursor(0, 0);
    if (!ready) {
        oled.println("LoRa init failed");
    } else if (!running) {
        oled.println("Waiting for GPS");
    } else {
        oled.println(done ? "DONE" : "RUNNING");
        oled.print("Time: ");
        oled.print((now - startTime) / 1000);
        oled.println(" / 120 s");
        oled.print("Check max: "); oled.println(checkMax);
        oled.print("TX max: "); oled.println(txMax);
        oled.println("Intervals in ms");
        oled.print("TX accepted: "); oled.println(sent);
        oled.println(done ? "Reset to repeat" : "");
    }
    oled.display();
}

void setup() {
    Wire.begin();
    if (!oled.begin(0x3C)) while (true) delay(100);
    oled.setTextSize(1);
    oled.setTextColor(SH110X_WHITE);
    oled.setTextWrap(false);
    show(millis());
    gpsInit();
    ready = loraInit(DEVICE_ID);
    randomSeed(DEVICE_ID);
    show(millis());
}

void loop() {
    if (!ready || done) return;

    unsigned long now = millis();
    if (running) {
        checkMax = max(checkMax, now - lastCheck);
        lastCheck = now;
    }
    gpsUpdate();
    loraUpdate();
    now = millis();

    if (!running) {
        if (!gpsHasFreshFix(now, 3000)) return;
        running = true;
        startTime = lastCheck = lastTx = lastAttempt = now;
    }

    if (now - startTime >= TEST_MS) {
        txMax = max(txMax, now - lastTx);
        done = true;
        show(now);
        return;
    }

    if (now - lastAttempt >= interval) {
        lastAttempt = now;

        interval = static_cast<unsigned long>(random(1800, 2201));
        GPSData gps = gpsGetData();
        if (gpsHasFreshFix(now, 3000) &&
            isfinite(gps.latitude) && isfinite(gps.longitude)) {
            LocationPacket packet = {DEVICE_ID, sequence++,
                                     gps.latitude, gps.longitude, 0};
            String payload = createLocationPacket(packet);
            unsigned long requestedAt = millis();
            if (loraSend(0, payload)) {

                txMax = max(txMax, requestedAt - lastTx);
                lastTx = requestedAt;
                ++sent;
            }
        }
    }

    now = millis();
  
    txMax = max(txMax, now - lastTx);
    if (now - lastDisplay >= 1000) {
        lastDisplay = now;
        show(now);
    }
}
