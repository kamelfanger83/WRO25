#pragma once

#include <cassert>
#include <cmath>
#include <iostream>

#include "geo/utils.cpp"
#include "structs.h"

template <typename T> T unwrap(std::optional<T> opt) {
  if (!opt.has_value()) {
    std::cerr << "Unwrapped None!";
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
    return Vector{
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

Vector normalize(const Vector &v) {
    double length = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    return Vector{v.x / length, v.y / length, v.z / length};
}
