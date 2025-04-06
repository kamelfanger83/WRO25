#pragma once

#include "../structs.h"

Vector Vector::operator+(const Vector &o) const {
  return {this->x + o.x, this->y + o.y, this->z + o.z};
}

Vector Vector::operator*(double f) const {
  return {this->x * f, this->y * f, this->z * f};
}
