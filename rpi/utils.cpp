#pragma once

#include <cassert>
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
