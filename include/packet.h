#pragma once

#include <Arduino.h>

// Location data exchanged between devices
struct LocationPacket {
    int source;
    unsigned int sequence;
    double latitude;
    double longitude;
    int hops;
};

String createLocationPacket(const LocationPacket& packet);
bool parseLocationPacket(const String& data, LocationPacket& packet);