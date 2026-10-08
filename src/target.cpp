#include "target.h"
#include "device.h"

namespace {
    constexpr unsigned int deviceCount = sizeof(devices) / sizeof(devices[0]);

    struct MemberLocation{
        Coordinates coordinates;
        unsigned long timeStamp;
        bool available;
    };

    MemberLocation locations[deviceCount] = {};

    int findDeviceIndex(uint16_t id) {
        for (unsigned int i = 0; i < deviceCount; ++i) {
            if (devices[i].id == id) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }
}

bool targetSet(uint16_t deviceId, const Coordinates& coordinates, unsigned long timeStamp)
{
    int index = findDeviceIndex(deviceId);
    if (index < 0 || deviceId == DEVICE_ID) {
        return false;
    }

    locations[index].coordinates = coordinates;
    locations[index].timeStamp = timeStamp;
    locations[index].available = true;
    return true;
}

bool targetGet(uint16_t deviceId, Coordinates& coordinates, unsigned long now, unsigned long maxAge)
{
    int index = findDeviceIndex(deviceId);

    if (index < 0 || !locations[index].available  || now - locations[index].timeStamp > maxAge) {
        return false;
    }

    coordinates = locations[index].coordinates;
    return true;
}
