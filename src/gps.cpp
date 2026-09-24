#include <Arduino.h>
#include <Adafruit_GPS.h>
#include <cstring>
#include "gps.h"

static Adafruit_GPS gps(&Serial1);
static GPSData currentGPS;

void gpsInit()
{
    gps.begin(9600);
    gps.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
    gps.sendCommand(PMTK_SET_NMEA_UPDATE_2HZ);
}

void gpsUpdate()
{
    while (gps.available() > 0) {
        gps.read();

        if (!gps.newNMEAreceived()) {
            continue;
        }

        char* sentence = gps.lastNMEA();

        if (strncmp(sentence, "$GNRMC,", 7) != 0 &&
            strncmp(sentence, "$GPRMC,", 7) != 0) {
            continue;
        }

        if (!gps.parse(sentence)) {
            continue;
        }

        currentGPS.fix = gps.fix;

        if (currentGPS.fix) {
            currentGPS.latitude = gps.latitudeDegrees;
            currentGPS.longitude = gps.longitudeDegrees;
            currentGPS.time =
                gps.hour * 10000 + gps.minute * 100 + gps.seconds;
        }
    }
}

GPSData gpsGetData()
{
    return currentGPS;
}
