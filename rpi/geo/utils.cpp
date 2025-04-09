#pragma once

#include "../structs.h"

Vector Vector::operator+(const Vector &o) const {
  return {this->x + o.x, this->y + o.y, this->z + o.z};
}

Vector Vector::operator*(double f) const {
  return {this->x * f, this->y * f, this->z * f};
}

Vector Vector::operator-(const Vector &o) const {
  return {this->x - o.x, this->y - o.y, this->z - o.z};
}

double Vector::operator*(const Vector &o) const {
  return this->x * o.x + this->y * o.y + this->z * o.z;
}

Vector crossP(const Vector &a, const Vector &b) {
  return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}
