#pragma once

#include <cassert>
#include <cmath>
#include <iostream>

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
  Vector d = {pose.x - this->x, pose.y - this->y, 0};
  float dotp = d * Vector{cos(pose.theta), sin(pose.theta), 0};
  if (dotp < 0 && d * d < 20) {
    std::cout << "We reached the waypoint within " << std::sqrt(d * d) << "cm."
              << std::endl;
    return true;
  }
  return false;
}
