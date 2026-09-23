#include "imu.h"
#include <Arduino.h>
#include <SparkFun_BNO08x_Arduino_Library.h>
#include <math.h>

#define IMU_RST       5
#define IMU_INT       6
#define IMU_ADDRESS   0x4B

static BNO08x imu;
static float heading = 0.0;
static float accuracy = 0.0;

static bool setReports()
{
    if (imu.enableRotationVector())
    {
        Serial.println("Rotation vector enabled");
        return true;
    }
    
    Serial.println("Unable to enable rotation vector");
    return false;
}

bool imuInit() {
    if(imu.begin(IMU_ADDRESS, Wire, IMU_INT, IMU_RST) == false) {
        return false;
    }

    if(setReports() == false) {
        return false;
    }

    return true;
}

void imuUpdate() {
    if(imu.getSensorEvent() == true) {
        if(imu.getSensorEventID() == SENSOR_REPORTID_ROTATION_VECTOR) {
            float quatI = imu.getQuatI();
            float quatJ = imu.getQuatJ();
            float quatK = imu.getQuatK();
            float quatReal = imu.getQuatReal();

            float yaw = atan2(
                2.0 * (quatReal * quatK + quatI * quatJ),
                1.0 - 2.0 * (quatJ * quatJ + quatK * quatK)
            );

            yaw = yaw * 180.0 / PI;

            heading = 90.0 - yaw;
            if(heading < 0) {
                heading += 360.0;
            }

            accuracy = imu.getQuatRadianAccuracy();
        }
    }

    if(imu.wasReset()) {
        if(setReports() == false) {
            Serial.println("Unable to restore rotation vector after IMU reset");
        }
    }
}

float imuGetHeading() {
    return heading;
}

float imuGetAccuracy() {
    return accuracy;
}