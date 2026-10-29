#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <Wire.h>
#include <Adafruit_SH110X.h>
#include "lora.h"

#define deviceAddress 1 // Load device B (2) and then change address and load to device A (1) to test the echo functionality.

static Adafruit_SH1106G oled(128, 64, &Wire);
static bool displayReady = false;

static void showStatus(const String& status,
                       const String& detail = "",
                       unsigned int replies = 0) {
    if (!displayReady) return;

    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(SH110X_WHITE);

    oled.setCursor(0, 0);
    oled.print("LORA TEST - ");
    oled.print(deviceAddress == 1 ? "A" : "B");

    oled.drawLine(0, 10, 127, 10, SH110X_WHITE);

    oled.setCursor(0, 16);
    oled.print(status);

    oled.setCursor(0, 30);
    oled.print(detail);

    if (deviceAddress == 1) {
        oled.setCursor(0, 46);
        oled.print("Replies: ");
        oled.print(replies);
        oled.print("/10");
    }

    oled.display();
}

#if deviceAddress != 1 && deviceAddress != 2
#error "deviceAddress must be 1 or 2"
#endif

static bool radioReady = false;

void setup() {
    Wire.begin();
    displayReady = oled.begin(0x3C);

    showStatus("Initializing...");

    radioReady = loraInit(deviceAddress);

    showStatus(
        radioReady ? "Radio ready" : "Radio init FAILED",
        deviceAddress == 1 ? "Starting test..." : "Waiting for A..."
    );
}

void loop() {
    loraUpdate();

#if deviceAddress == 1
    static unsigned int packetNumber = 1;
    static unsigned int repliesReceived = 0;
    static bool waitingForReply = false;
    static bool finished = false;
    static String expectedReply;
    static unsigned long sentAt = 0;

    if (finished) return;

    if (!radioReady) {
        showStatus("FAIL", "Radio init failed");
        finished = true;
        return;
    }

    if (!waitingForReply &&
        (packetNumber == 1 || millis() - sentAt >= 1000)) {

        LoRaMessage oldMessage;
        loraReceive(oldMessage);

        expectedReply = "TEST";
        expectedReply += packetNumber;

        showStatus("Sending", expectedReply, repliesReceived);
        sentAt = millis();

        if (loraSend(2, expectedReply)) {
            waitingForReply = true;
            showStatus("Waiting for echo", expectedReply,
                       repliesReceived);
        } else {
            showStatus("Send rejected", expectedReply,
                       repliesReceived);
            packetNumber++;
        }
    }

    if (waitingForReply) {
        LoRaMessage message;

        if (loraReceive(message) &&
            message.sender == 2 &&
            message.payload == (String("ACK:") + expectedReply)) {

            repliesReceived++;
            packetNumber++;
            waitingForReply = false;

            showStatus("Reply received", message.payload,
                       repliesReceived);
        } else if (millis() - sentAt >= 5000) {
            packetNumber++;
            waitingForReply = false;

            showStatus("TIMEOUT", expectedReply, repliesReceived);
        }
    }

    if (packetNumber > 10) {
        showStatus(repliesReceived == 10 ? "PASS" : "FAIL",
                   "Test complete", repliesReceived);
        finished = true;
    }

#else
    if (!radioReady) return;

    LoRaMessage message;

    if (loraReceive(message) && message.sender == 1) {
        bool validPacket = false;

        for (unsigned int number = 1; number <= 10; number++) {
            if (message.payload == (String("TEST") + number)) {
                validPacket = true;
                break;
            }
        }

        if (validPacket) {
            showStatus("Received", message.payload);

            String reply = String("ACK:") + message.payload;
            bool accepted = loraSend(1, reply);

            showStatus(accepted ? "Echo accepted" : "Echo failed",
                       reply);
        }
    }
#endif
}
