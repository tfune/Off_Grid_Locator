#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include "lora.h"

#define deviceAddress 2 // Load device B (2) and then change address and load to device A (1) to test the echo functionality.

#if deviceAddress != 1 && deviceAddress != 2
#error "deviceAddress must be 1 or 2"
#endif

static bool radioReady = false;

void setup()
{
    Serial.begin(115200);

    while (!Serial) {
        delay(10);
    }

    Serial.print("Echo test version 2 — Device ");
    Serial.println(deviceAddress == 1 ? "A" : "B");

    radioReady = loraInit(deviceAddress);

    if (Serial) {
        Serial.println(
            radioReady ? "LoRa initialized" : "LoRa initialization failed"
        );
    }
}

void loop()
{
    delay(5);
    loraUpdate();

#if deviceAddress == 1
    static unsigned int packetNumber = 1;
    static unsigned int repliesReceived = 0;    
    static bool waitingForReply = false;
    static bool finished = false;
    static String expectedReply;
    static unsigned long sentAt = 0;

    if(finished) return;

    if (!radioReady) {
        Serial.println("FAIL: A radio initialization failed");
        finished = true;
        return;
    }

    if (!waitingForReply && (packetNumber == 1 || millis() - sentAt >= 1000)) {
        
        LoRaMessage oldMessage;
        loraReceive(oldMessage);

        expectedReply = "TEST";
        expectedReply += packetNumber;

        Serial.print("A sending: ");
        Serial.println(expectedReply);

        sentAt = millis();

        bool accepted = loraSend(2, expectedReply);
        
        if (accepted) {
            waitingForReply = true;
        }
        else {
            Serial.println("FAIL: local send acknowledgment");
            packetNumber++;
        }
    }

    if (waitingForReply) {
        LoRaMessage message;

        if (loraReceive(message) &&
            message.sender == 2 &&
            message.payload == (String("ACK:") + expectedReply)) {

            Serial.print("A received: ");
            Serial.println(message.payload);

            repliesReceived++;
            packetNumber++;
            waitingForReply = false;
        }
        else if (millis() - sentAt >= 5000) {
            Serial.print("TIMEOUT: ");
            Serial.println(expectedReply);
 
            packetNumber++;
            waitingForReply = false;
        }
    }

    if (packetNumber > 10) {
        Serial.print("Echoes received: ");
        Serial.print(repliesReceived);
        Serial.println("/10");

        Serial.println(
            repliesReceived == 10 ? "PASS" : "FAIL"
        );

        finished = true;
    }

#else
    if (!radioReady) return;

    LoRaMessage message;

    if (loraReceive(message) && message.sender == 1) {
        // Accept only the ten exact test messages.
        bool validPacket = false;

        for (unsigned int number = 1; number <= 10; number++) {
            String expected = "TEST";
            expected += number;

            if (message.payload == expected) {
                validPacket = true;
                break;
            }
        }

        if (validPacket) {
            Serial.print("B received: ");
            Serial.println(message.payload);
            Serial.flush();

            String reply = "ACK:";
            reply += message.payload;
            bool accepted = loraSend(1, reply);
            Serial.println(accepted ? "Echo accepted" : "Echo failed");
            Serial.flush();
        }
    }
#endif
}
