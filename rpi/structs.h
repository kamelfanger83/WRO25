#pragma once
#include <optional>
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

/// Represents on a Frame captured by the camera.
/// angle: angle of the direction of the line. In the range of [0, pi ). For a
///     given angle a, the direction vector of the line is (cos(a), sin(a)).
/// distanceToOrigin: distance of the line from the origin (0, 0) (top left).
struct ScreenLine {
  double angle;
  double distanceToOrigin;
};

struct ScreenLineSet{
  std::optional<ScreenLine> blue, orange, right, back, left;
};

/// Represents commands for the motors.
/// Acceptable ranges:
///     - 0 <= angle <= 170
///     - -255 <= speed <= 255
struct Commands {
  int angle;
  int speed;
};
