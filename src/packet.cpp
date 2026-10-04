#include "packet.h"

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

    packet.source = data.substring(comma1 + 1, comma2).toInt();
    packet.sequence = data.substring(comma2 + 1, comma3).toInt();
    packet.latitude = atof(data.substring(comma3 + 1, comma4).c_str());
    packet.longitude = atof(data.substring(comma4 + 1, comma5).c_str());
    packet.hops = data.substring(comma5 + 1).toInt();

    return true;
}