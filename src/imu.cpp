#include "imu.h"
#include <Arduino.h>
#include <SparkFun_BNO08x_Arduino_Library.h>
#include <math.h>

#define IMU_RST       5
#define IMU_INT       6
#define IMU_ADDRESS   0x4B

BNO08x imu;
static float heading = 0.0;

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

            float yaw = atan2(
                2.0 * (quatReal * quatK + quatI * quatJ),
                1.0 - 2.0 * (quatJ * quatJ + quatK * quatK)
            );

            yaw = yaw * 180.0 / PI;

            heading = 90.0 - yaw;
            if(heading < 0) {
                heading += 360.0;
            }
        }
    }

    if(imu.wasReset()) {
        setReports();
    }
}

float IMU_GetHeading() {
    return heading;
}