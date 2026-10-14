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

bool imuInit() {
    if (!imu.begin(IMU_ADDRESS, Wire, IMU_INT, IMU_RST)) {
        return false;
    }

    imu.enableRotationVector();

    return true;
}

void imuUpdate() {
    if (imu.getSensorEvent()) {
        if (imu.getSensorEventID() == SENSOR_REPORTID_ROTATION_VECTOR) {
            float quatI = imu.getQuatI();
            float quatJ = imu.getQuatJ();
            float quatK = imu.getQuatK();
            float quatReal = imu.getQuatReal();

            // Convert the rotation-vector quaternion into yaw
            float yaw = atan2(
                2.0 * (quatReal * quatK + quatI * quatJ),
                1.0 - 2.0 * (quatJ * quatJ + quatK * quatK)
            );

            // Convert yaw from radians to degrees
            yaw = yaw * 180.0 / PI;

            // Apply the IMU mounting offset and normalize heading to 0-360 degrees
            constexpr float mountingOffset = 180.0;
            heading = fmod(mountingOffset - yaw + 360.0, 360.0);

            accuracy = imu.getQuatRadianAccuracy();
        }
    }

    // Re-enable rotation-vector reports if the IMU resets
    if (imu.wasReset()) {
        imu.enableRotationVector();
    }
}

float imuGetHeading() {
    return heading;
}

float imuGetAccuracy() {
    return accuracy;
}