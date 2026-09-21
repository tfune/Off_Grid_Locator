#include "imu.h"
#include <Arduino.h>
#include <SparkFun_BNO08x_Arduino_Library.h>

#define IMU_RST       5
#define IMU_INT       6
#define IMU_ADDRESS   0x4B

BNO08x imu;

void setReports()
{
    Serial.println("Setting reports...");

    if (imu.enableRotationVector())
    {
        Serial.println("Rotation vector enabled");
    }
    else
    {
        Serial.println("Unable to enable rotation vector");
    }
}

bool IMU_Init() {
    if(imu.begin(IMU_ADDRESS, Wire, IMU_INT, IMU_RST) == false) {
        return false;
    }

    setReports();

    return true;
}

void IMU_Update() {

    if(imu.getSensorEvent() == true) {
        if(imu.getSensorEventID() == SENSOR_REPORTID_ROTATION_VECTOR) {
            float quatI = imu.getQuatI();
            float quatJ = imu.getQuatJ();
            float quatK = imu.getQuatK();
            float quatReal = imu.getQuatReal();
            float quatAccuracy = imu.getQuatRadianAccuracy();

            static unsigned long lastPrint = 0;

            if(millis() - lastPrint >= 500) {
                Serial.print("I: ");
                Serial.print(quatI, 3);
                Serial.print(" J: ");
                Serial.print(quatJ, 3);
                Serial.print(" K: ");
                Serial.print(quatK, 3);
                Serial.print(" Real: ");
                Serial.print(quatReal, 3);
                Serial.print(" Accuracy: ");
                Serial.println(quatAccuracy, 3);

                lastPrint = millis();
            }
        }
    }

    if(imu.wasReset()) {
        setReports();
    }
}