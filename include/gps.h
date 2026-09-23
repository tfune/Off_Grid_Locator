#pragma once

struct GPSData {
    bool fix;
    double latitude;
    double longitude;
    int time;
};

void gps_init();
void gps_update();
GPSData gps_get_data();