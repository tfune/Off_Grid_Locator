#include "navigation.h"
#include <math.h>

namespace{ 
    constexpr double pi = 3.14159265358979323846;
    constexpr double earthsRadius = 6371000.0;
}

double toRadians(double degrees){
    return degrees * pi / 180.0;
}

double toDegrees(double radians){
    return radians * 180.0 / pi;
}

double normalizeDegrees(double degrees){
    double result = fmod(degrees, 360.0);
    return result < 0 ? result + 360.0 : result;
}

NavigationResult calculateNavigation(const Coordinates& current, const Coordinates& target) {
    const double lat1 = toRadians(current.latitude);
    const double lat2 = toRadians(target.latitude);
    const double deltaLat = lat2 - lat1;
    const double deltaLon =
        toRadians(target.longitude - current.longitude);

    const double sinHalfLat = sin(deltaLat / 2.0);
    const double sinHalfLon = sin(deltaLon / 2.0);

    double a =
        sinHalfLat * sinHalfLat +
        cos(lat1) * cos(lat2) * sinHalfLon * sinHalfLon;

    if (a < 0.0) a = 0.0;
    if (a > 1.0) a = 1.0;

    const double distance =
        earthsRadius *
        2.0 * atan2(sqrt(a), sqrt(1.0 - a));

    const double y = sin(deltaLon) * cos(lat2);
    const double x =
        cos(lat1) * sin(lat2) -
        sin(lat1) * cos(lat2) * cos(deltaLon);

    const bool bearingValid = hypot(x, y) > 1e-12;

    const double bearing = bearingValid
        ? normalizeDegrees(toDegrees(atan2(y, x)))
        : 0.0;

    return {target, distance, bearing, bearingValid};
}

float calculateArrowAngle(float bearingDegrees, float headingDegrees) {
    return static_cast<float>(normalizeDegrees(static_cast<double>(bearingDegrees) - headingDegrees));
}