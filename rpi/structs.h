#pragma once
#include <cstdint>
#include <optional>

// #define WIDTH 1280
// #define HEIGHT 720
#define WIDTH 1920
#define HEIGHT 1080

/// Represents either a position in 3D space (Ortsvektor) or just a vector
/// without or with another intended startpoint.
struct Vector {
  double x;
  double y;
  double z;
  Vector operator+(const Vector &o) const;
  Vector operator*(double f) const;
  Vector operator-(const Vector &o) const;
  double operator*(const Vector &o) const;
};

/// Describes a plane by a normal direction and a d. The plane that is described
/// is the set of all points v with v * n = d where n is the normal vector and *
/// is the dot product.
struct Plane {
  Vector normal;
  double d;
};

/// Represents a position on the camera screen in pixels. As usual, (0, 0) is
/// top left.
struct ScreenPosition {
  double x;
  double y;
};

/// Represents the current position and orientation of the camera and with it
/// the Robot.
/// theta: angle. As usual, 0 means positive x and counterclockwise is the
///     positive direction.
struct Pose {
  double x;
  double y;
  double theta;

  Pose operator+(const Pose &o) const;
  Pose operator*(double f) const;

  Pose operator/(const Pose &o) const;
  Pose operator*(const Pose &o) const;
};

/// Represents a point on that field that is scheduled to be visited.
struct Waypoint {
  double x;
  double y;
  // double theta; Brauchen wir maybe später

  /// returns if a position has reached the waypoint
  bool reached(Pose p);
};

struct HSVPixel {
  uint8_t h, s, v;
};

struct Frame {
  HSVPixel *HSV;
  long long timestamp;
};

/// Represents on a Frame captured by the camera.
/// angle: angle of the direction of the line. In the range of [0, pi ). For a
///     given angle a, the direction vector of the line is (cos(a), sin(a)).
/// distanceToOrigin: distance of the line from the origin (0, 0) (top left).
struct ScreenLine {
  double angle;
  double distanceToOrigin;
};

struct ScreenLineSet {
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

/// State of the controller for PD control
struct ControllerState {
  double prev_error = 0;
};
