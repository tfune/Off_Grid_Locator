#pragma once

#include <Arduino.h>

// LoRa message structure.
struct LoRaMessage {
    uint16_t sender;
    String payload;
    int rssi;
    int snr;
};


bool loraInit(uint16_t address); // Initialize the LoRa module with the given address. Returns true on success, false on failure.
bool loraIsInitialized(); // Check if the LoRa module is initialized. Returns true if initialized, false otherwise.

void loraUpdate();

bool loraSend(uint16_t destination, const String& data); // Send a message to the specified destination address. Returns true if the message was accepted for sending, false otherwise.

bool loraReceive(LoRaMessage& message); // Check if a message has been received. If a message is available the function returns true. If no message is available, the function returns false.
