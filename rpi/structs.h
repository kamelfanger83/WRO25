#pragma once

struct Pose {
    double x;
    double y;
    double theta;
};

struct Waypoint {
    double x;
    double y;
    // double theta; Brauchen wir maybe später
};

/// Acceptable ranges:
///     - 0 <= angle <= 170
///     - -255 <= speed <= 255
struct Commands {
    int angle;
    int speed;
};