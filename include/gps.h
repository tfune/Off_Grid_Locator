#pragma once

struct GPSData {
    bool fix;
    double latitude;
    double longitude;
    int time;
};

bool gpsInit();
void gpsUpdate();
GPSData gpsGetData();
bool gpsHasFreshFix(unsigned long now, unsigned long timeStamp);
bool gpsIsCommunicating();