#pragma once

#include <cassert>
#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>

#include "camera/camera.h"
#include "camera/find_color.cpp"
#include "geo/utils.cpp"
#include "structs.h"

template <typename T> T unwrap(std::optional<T> opt) {
  if (!opt.has_value()) {
    std::cout << "Unwrapped None!";
    assert(false);
  }
  return *opt;
}

Pose Pose::operator+(const Pose &o) const {
  return {this->x + o.x, this->y + o.y, this->theta + o.theta};
}

Pose Pose::operator*(double f) const {
  return {this->x * f, this->y * f, this->theta * f};
}

void printPose(const Pose &pose) {
  std::cout << "Pose(x=" << pose.x << ", y=" << pose.y
            << ", theta=" << pose.theta << ")" << std::endl;
}

bool Waypoint::reached(Pose pose) {
  Vector d = {this->x - pose.x, this->y - pose.y, 0};
  float dotp = d * Vector{cos(pose.theta), sin(pose.theta), 0};
  if (dotp < 0 && d * d < 900) {
    std::cout << "We reached the waypoint within " << std::sqrt(d * d) << "cm."
              << std::endl;
    return true;
  }
  return false;
}

Pose Pose::operator/(const Pose &o) const {
  float mx = this->x - o.x;
  float my = this->y - o.y;
  float rotx = mx * std::cos(-o.theta) - my * std::sin(-o.theta);
  float roty = mx * std::sin(-o.theta) + my * std::cos(-o.theta);
  return {rotx, roty, this->theta - o.theta};
}

Pose Pose::operator*(const Pose &o) const {
  float rotx = o.x * std::cos(this->theta) - o.y * std::sin(this->theta);
  float roty = o.x * std::sin(this->theta) + o.y * std::cos(this->theta);
  return {this->x + rotx, this->y + roty, this->theta + o.theta};
}

Vector cross(const Vector &a, const Vector &b) {
  return Vector{a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
                a.x * b.y - a.y * b.x};
}

Vector normalize(const Vector &v) {
  double length = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
  return Vector{v.x / length, v.y / length, v.z / length};
}

/// Returns screenLine from the given two points. If piPeriod = true, returns in
/// angle in [0, M_PI).
ScreenLine lineFromPoints(const ScreenPosition &p1, const ScreenPosition &p2,
                          bool piPeriod = true) {
  auto Δx = (p2).x - (p1).x;
  auto Δy = (p2).y - (p1).y;

  double angle = std::atan2(Δy, Δx); // Calculate angle of line
  if (angle < 0 && piPeriod)
    angle += M_PI;

  double distanceToOrigin =
      -std::sin(angle) * (p1).x + std::cos(angle) * (p1).y;

  return ScreenLine{angle, distanceToOrigin};
}

void captureFrameBlocking() {
  long long lastTimeStamp = lastFrame.timestamp;
  queueCapture();
  while (lastTimeStamp == lastFrame.timestamp) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

bool isWhiteOrWood(const HSVPixel pixel) {
  return isWhite(pixel) || isWood(pixel);
}

bool isFlipped(const Frame &frame) {
  auto whitePoints = mask(frame, isWhiteOrWood);

  int lbox = 0, rbox = 0;
  for (const auto &p : whitePoints) {
    if (415 < p.y && p.y < 715) {
      if (p.x < 200)
        ++lbox;
      if (WIDTH - 200 < p.x)
        ++rbox;
    }
  }

  bool flipped;
  if (lbox > rbox)
    flipped = true;
  else
    flipped = false;

  if (flipped)
    std::cout << "We are going counterclockwisely" << std::endl;
  else
    std::cout << "We are going clockwisely" << std::endl;

  Frame debug = cloneFrame(lastFrame);
  colorColor(debug, whitePoints, {0, 255, 255});

  debug.timestamp = 69;
  saveFrame(debug);

  return flipped;
}
