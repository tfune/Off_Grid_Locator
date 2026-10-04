#pragma once

#include <Arduino.h>

struct LocationPacket {
    int source;
    unsigned int sequence;
    double latitude;
    double longitude;
    int hops;
};

// Creates a location packet string for transmission
String createLocationPacket(const LocationPacket& packet);

// Parses a received location packet string
bool parseLocationPacket(const String& data, LocationPacket& packet);