#include "imu.h"
#include <Arduino.h>
#include <SparkFun_BNO08x_Arduino_Library.h>
#include <math.h>

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

            float roll = atan2(
                2.0 * (quatReal * quatI + quatJ * quatK),
                1.0 - 2.0 * (quatI * quatI + quatJ * quatJ)
            );

            roll = roll * 180.0 / PI;

            float pitch = asin(2.0 * (quatReal * quatJ - quatK * quatI));
            pitch = pitch * 180.0 / PI;

            float yaw = atan2(
                2.0 * (quatReal * quatK + quatI * quatJ),
                1.0 - 2.0 * (quatJ * quatJ + quatK * quatK)
            );

            yaw = yaw * 180.0 / PI;
            if(yaw < 0) {
                yaw += 360.0;
            }

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

                Serial.print(" Roll: ");
                Serial.print(roll, 1);
                Serial.print(" Pitch: ");
                Serial.print(pitch, 1);
                Serial.print(" Yaw: ");
                Serial.println(yaw, 1);

                lastPrint = millis();
            }
        }
    }

    if(imu.wasReset()) {
        setReports();
    }
}