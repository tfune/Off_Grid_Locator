#pragma once

struct GPSData {
    bool fix;
    double latitude;
    double longitude;
    int time;
};

void gpsInit();
void gpsUpdate();
GPSData gpsGetData();