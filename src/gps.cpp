#include <Arduino.h>
#include <Adafruit_GPS.h>
#include <cstring>
#include "gps.h"

static Adafruit_GPS gps(&Serial1);
static GPSData currentGPS;
static unsigned long lastGPSUpdate = 0;
static bool gpsUpdateReceived = false;
static bool filterReady = false;
constexpr double gpsAlpha = 0.3;
constexpr unsigned long filtRstGap = 3000;

bool gpsInit()
{
    gps.begin(9600);
    gps.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
    gps.sendCommand(PMTK_SET_NMEA_UPDATE_10HZ);

    unsigned long startTime = millis();

    while (millis() - startTime < 5000) {
        gpsUpdate();

        if (gpsIsCommunicating()) {
            return true;
        }
    }

    return false;
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
        
        const unsigned long now = millis();

        if (!gpsUpdateReceived || now - lastGPSUpdate > filtRstGap) {
            filterReady = false;
        }

        lastGPSUpdate = now;
        gpsUpdateReceived = true;
        currentGPS.fix = gps.fix;

        if (!currentGPS.fix) {
            filterReady = false;
            continue;
        }

        const double latitude = gps.latitudeDegrees;
        const double longitude = gps.longitudeDegrees;

        if (!filterReady) {
            currentGPS.latitude = latitude;
            currentGPS.longitude = longitude;
            filterReady = true;
        } else {
            currentGPS.latitude += gpsAlpha * (latitude - currentGPS.latitude);

            currentGPS.longitude += gpsAlpha * (longitude - currentGPS.longitude);
        }

        currentGPS.time =
            gps.hour * 10000 + gps.minute * 100 + gps.seconds;
    }
}

GPSData gpsGetData()
{
    return currentGPS;
}

bool gpsHasFreshFix(unsigned long now, unsigned long timeStamp)
{
        return gpsUpdateReceived && currentGPS.fix && now - lastGPSUpdate <= timeStamp;
}

bool gpsIsCommunicating()
{
   constexpr unsigned long  comTimeOut = 5000;
   return gpsUpdateReceived && millis() - lastGPSUpdate <= comTimeOut;
}
