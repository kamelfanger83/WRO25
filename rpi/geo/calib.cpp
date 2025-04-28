#include <cmath>
#include <iostream>

#include "../structs.h"
#include "camera.cpp"

int main() {
  double xCorner, yCorner;
  std::cin >> xCorner >> yCorner;

  Pose pose{50, 150, M_PI_2};
  Vector corner = getStartEndPoints(Line::BORDER_OUT_1).second;

  CoordinateSystem cameraSystem = getCameraSystem(pose);
  Vector unproj =
      unprojectPoint(cameraSystem, {xCorner, yCorner}) - cameraSystem.origin;
  Vector rel = corner - cameraSystem.origin;

  unproj = unproj * (1 / std::sqrt(unproj * unproj));
  rel = rel * (1 / std::sqrt(rel * rel));

  double unprojTilt = asin(unproj.z);
  double relTilt = asin(rel.z);
  double unprojTheta = atan2(unproj.x, unproj.y);
  double relTheta = atan2(rel.x, rel.y);

  auto proj = *projectPoint(cameraSystem, corner);

  std::cout << "Projected corner: " << proj.x << " " << proj.y << std::endl;

  std::cout << "unproj len: " << sqrt(unproj * unproj);

  std::cout << "Corrected tilt: " << TILT_ANG + unprojTilt - relTilt
            << "\nCorrected theta: " << THETA_OFFSET + unprojTheta - relTheta
            << std::endl;
}
