#pragma once

/// Represents either a position in 3D space (Ortsvektor) or just a vector
/// without or with another intended startpoint.
struct Vector {
  double x;
  double y;
  double z;
};

/// Represents the current position and orientation of the camera and with it
/// the Robot.
struct Pose {
  double x;
  double y;
  double theta;
};

/// Represents a point on that field that is scheduled to be visited.
struct Waypoint {
  double x;
  double y;
  // double theta; Brauchen wir maybe später
};

/// Represents commands for the motors.
/// Acceptable ranges:
///     - 0 <= angle <= 170
///     - -255 <= speed <= 255
struct Commands {
  int angle;
  int speed;
};
