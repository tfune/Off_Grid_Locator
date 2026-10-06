#include "packet.h"

// Creates a location packet string for LoRa transmission
String createLocationPacket(const LocationPacket& packet) {
    String data = "LOC,";
    data += String(packet.source);
    data += ",";
    data += String(packet.sequence);
    data += ",";
    data += String(packet.latitude, 6);
    data += ",";
    data += String(packet.longitude, 6);
    data += ",";
    data += String(packet.hops);

    return data;
}

// Parses and validates a received location packet
bool parseLocationPacket(const String& data, LocationPacket& packet) {
    if(!data.startsWith("LOC,")) {
        return false;
    }

    int comma1 = data.indexOf(',');
    int comma2 = data.indexOf(',', comma1 + 1);
    int comma3 = data.indexOf(',', comma2 + 1);
    int comma4 = data.indexOf(',', comma3 + 1);
    int comma5 = data.indexOf(',', comma4 + 1);

    if(comma1 == -1 || comma2 == -1 || comma3 == -1 || comma4 == -1 || comma5 == -1) {
        return false;
    }

    String sourceString = data.substring(comma1 + 1, comma2);
    String sequenceString = data.substring(comma2 + 1, comma3);
    String latitudeString = data.substring(comma3 + 1, comma4);
    String longitudeString = data.substring(comma4 + 1, comma5);
    String hopsString = data.substring(comma5 + 1);

    char *end;

    // Parse source
    const char *start = sourceString.c_str();
    long source = strtol(start, &end, 10);

    if(end == start || *end != '\0' || source < 0) {
        return false;
    }

    // Parse sequence number
    start = sequenceString.c_str();
    unsigned long sequence = strtoul(start, &end, 10);

    if(end == start || *end != '\0' || sequenceString[0] == '-') {
        return false;
    }

    // Parse latitude
    start = latitudeString.c_str();
    double latitude = strtod(start, &end);

    if(end == start || *end != '\0' ||
        latitude < -90.0 || latitude > 90.0) {
        return false;
    }

    // Parse longitude
    start = longitudeString.c_str();
    double longitude = strtod(start, &end);

    if(end == start || *end != '\0' ||
        longitude < -180.0 || longitude > 180.0) {
        return false;
    }

    // Parse hops
    start = hopsString.c_str();
    long hops = strtol(start, &end, 10);

    if (end == start || *end != '\0' || hops < 0) {
        return false;
    }

    packet.source = source;
    packet.sequence = sequence;
    packet.latitude = latitude;
    packet.longitude = longitude;
    packet.hops = hops;

    return true;
}