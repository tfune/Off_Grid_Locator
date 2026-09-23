#include <Arduino.h>
#include <Adafruit_GPS.h>
#include <cstring>
#include "gps.h"

static Adafruit_GPS gps(&Serial1);
static GPSData currentGPS;

void gps_init()
{
    gps.begin(9600);
    gps.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
    gps.sendCommand(PMTK_SET_NMEA_UPDATE_2HZ);
}

void gps_update()
{
    gps.read();

    if (!gps.newNMEAreceived()) {
        return;
    }

    char* sentence = gps.lastNMEA();

    if (strncmp(sentence, "$GNRMC,", 7) != 0) {
        return;
    }

    if(!gps.parse(sentence)) {
        return;
    }

    currentGPS.fix = gps.fix;

    if (currentGPS.fix) {
        currentGPS.latitude = gps.latitudeDegrees;
        currentGPS.longitude = gps.longitudeDegrees;
        currentGPS.time = gps.hour * 10000 + gps.minute * 100 + gps.seconds;
    }
}

GPSData gps_get_data()
{
    return currentGPS;
}
