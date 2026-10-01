#pragma once

struct Coordinates { //A struct to hold latitude and longitude values
    double latitude;
    double longitude;
};

struct NavigationResult { //A struct to hold the results of navigation calculations
    Coordinates destination;
    double distance;
    double bearing;
    bool bearingValid;
};

NavigationResult calculateNavigation( //A function to calculate navigation information between two coordinates
    const Coordinates& current,
    const Coordinates& target
);

float calculateArrowAngle(float bearing, float currentHeading); //A function to calculate the angle for an arrow based on bearing and current heading