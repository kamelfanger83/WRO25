#pragma once

#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "../camera/draw_line.cpp"
#include "../structs.h"
#include "../utils.cpp"
#include "constants.h"
#include "coordinates.cpp"
#include "utils.cpp"

/// Describes another coordinate system relative to the standard coordinate
/// system. x, y, z form an orthonormal basis and point in the positive x, y, z
///     direction of the coordinate system.
/// origin is the standard location of the point (0, 0, 0) in the coordinate
/// system.
struct CoordinateSystem {
  Vector x, y, z, origin;
};

/// Rotates v around axis by given angle in radians counterclockwise.
Vector rotateAroundAxis(const Vector &v, const Vector &axis,
                        double angleRadians) {
  Vector k = normalize(axis);
  double cosTheta = std::cos(angleRadians);
  double sinTheta = std::sin(angleRadians);

  Vector term1 = v * cosTheta;
  Vector term2 = cross(k, v) * sinTheta;
  Vector term3 = k * (k * v) * (1 - cosTheta);

  return term1 + term2 + term3;
}

CoordinateSystem getCameraSystem(const Pose &pose) {
  Vector x{std::cos(pose.theta + THETA_OFFSET - M_PI_2f64),
           std::sin(pose.theta + THETA_OFFSET - M_PI_2f64), 0};
  Vector y{-std::sin(TILT_ANG) * std::cos(pose.theta + THETA_OFFSET),
           -std::sin(TILT_ANG) * std::sin(pose.theta + THETA_OFFSET),
           -std::cos(TILT_ANG)};
  Vector z{std::cos(TILT_ANG) * std::cos(pose.theta + THETA_OFFSET),
           std::cos(TILT_ANG) * std::sin(pose.theta + THETA_OFFSET),
           -std::sin(TILT_ANG)};
  x = rotateAroundAxis(x, z, -CAMERA_TWIST);
  y = rotateAroundAxis(y, z, -CAMERA_TWIST);
  Vector origin{pose.x, pose.y, CAM_HEIGHT};
  return {x, y, z, origin};
}

/// Unprojects a screen point to a 3D position with depth (z-coordinate in the
/// camera system) 1.
Vector unprojectPoint(const CoordinateSystem &cameraSystem,
                      const ScreenPosition &point) {
  double horizontalRange = 2. * std::tan(HORIZONTAL_FOV / 2);
  double verticalRange = horizontalRange / WIDTH * HEIGHT;
  double cameraX = (point.x - double(WIDTH) / 2.) / WIDTH * horizontalRange;
  double cameraY = (point.y - double(HEIGHT) / 2.) / HEIGHT * verticalRange;
  return cameraSystem.origin + cameraSystem.z + cameraSystem.x * cameraX +
         cameraSystem.y * cameraY;
}

/// Projects a given 3D point onto the camera Screen and returns location on the
/// screen. Returns None if the point is behind the camera.
std::optional<ScreenPosition> projectPoint(const CoordinateSystem &cameraSystem,
                                           const Vector &point) {
  Vector camVec = point - cameraSystem.origin;
  double depth = camVec * cameraSystem.z;
  if (depth <= 0)
    return {};
  double cameraX = (camVec * cameraSystem.x) / depth;
  double cameraY = (camVec * cameraSystem.y) / depth;
  double horizontalRange = 2. * std::tan(HORIZONTAL_FOV / 2);
  double verticalRange = horizontalRange / WIDTH * HEIGHT;
  double x = cameraX / horizontalRange * WIDTH + double(WIDTH) / 2.;
  double y = cameraY / verticalRange * HEIGHT + double(HEIGHT) / 2.;
  return {{x, y}};
}

std::optional<ScreenLine> projectLine(const Pose &pose, Line line,
                                      bool piPeriod = true) {

  auto camerasys = getCameraSystem(pose);

  auto [s, e] = getStartEndPoints(line);

  auto p1 = projectPoint(camerasys, s);
  auto p2 = projectPoint(camerasys, e);
  if (p1.has_value() && p2.has_value()) {
    return {lineFromPoints(*p1, *p2, piPeriod)};
  }
  if (!p1.has_value() && !p2.has_value())
    return {};
  bool flipped = false;
  if (p2.has_value()) {
    std::swap(s, e);
    flipped = true;
  }

  Vector v{s.x - e.x, s.y - e.y, s.z - e.z};
  e = e + v * ((camerasys.z * camerasys.origin - camerasys.z * e) /
                   (v * camerasys.z) +
               1 / std::sqrt(v * v));

  p1 = projectPoint(camerasys, s);
  p2 = projectPoint(camerasys, e);
  auto screenLine = lineFromPoints(unwrap(p1), unwrap(p2), piPeriod);
  if (!piPeriod && flipped) {
    screenLine.angle += M_PI;
    // ideally we'd also bring it in the range of [-PI, PI) or something but who
    // cares.
  }
  return {screenLine};
}

void drawProjectedLine(Frame &frame, const Pose &pose, Line line,
                       HSVPixel color) {

  auto camerasys = getCameraSystem(pose);
  auto [s, e] = getStartEndPoints(line);

  bool inRange = false;
  auto se = e - s;
  for (double t = 0; t < 1. + 1e-5; t += 5. / std::sqrt(se * se)) {
    Vector v = s * (1. - t) + e * t;
    if (auto projected = projectPoint(camerasys, v)) {
      bool ons = projected->x >= 0 && projected->x < WIDTH &&
                 projected->y >= 0 && projected->y < HEIGHT;
      inRange |= ons;
      if (ons) {
        // draw small circle at projected point
        int x = std::round(projected->x);
        int y = std::round(projected->y);
        for (int i = -2; i <= 2; ++i) {
          for (int j = -2; j <= 2; ++j) {
            if (i * i + j * j <= 4) {
              if (x + i >= 0 && x + i < WIDTH && y + j >= 0 && y + j < HEIGHT) {
                frame.HSV[(y + j) * WIDTH + (x + i)] = color;
              }
            }
          }
        }
      }
    }
  }

  if (inRange)
    // Unwrapping is fine because we know that if the line is in range, surely
    // one point has to be in front of the camera.
    drawLineInFrame(frame, unwrap(projectLine(pose, line)), color);
}

void drawProjectedLines(Frame &frame, Pose &pose) {

  std::vector<std::pair<std::array<Line, 4>, HSVPixel>> linesColors{
      {outerLines, {180, 255, 30}},
      {innerLines, {180, 255, 255}},
      {blueLines, {180, 255, 255}},
      {orangeLines, {0, 255, 255}}};

  for (auto [lines, color] : linesColors) {
    for (auto line : lines) {
      drawProjectedLine(frame, pose, line, color);
    }
  }
}

/// Returns two arbitrary points on a given ScreenLine.
std::pair<ScreenPosition, ScreenPosition>
pointsOnLine(const ScreenLine &screenLine) {
  double x1 = -sin(screenLine.angle) * screenLine.distanceToOrigin;
  double y1 = cos(screenLine.angle) * screenLine.distanceToOrigin;
  double x2 = x1 + 100 * cos(screenLine.angle);
  double y2 = y1 + 100 * sin(screenLine.angle);
  return {{x1, y1}, {x2, y2}};
}

// Temporary testing functions
void printVector(const std::string &name, const Vector &v) {
  std::cout << std::fixed << std::setprecision(3);
  std::cout << "  " << name << ": (" << v.x << ", " << v.y << ", " << v.z
            << ")\n";
}

/// Returns the plane that corresponds to a ScreenLine. By correspond I mean
/// that points on that ScreenLine could come from anywhere on that plane. If
/// you think about what shape you obtain by "unprojecting" a line on a camera
/// screen, you get a plane. (beste doc comment)
// Call function above to get the screen position of two points on the line.
// Then unproject these two points. Then take the cross product between the
// vectors from the camera origin to these points to obtain the normal vector.
// Calculate d by tking the dotproduct of the camera origin with the normal
// vector as we know that the camera origin is on the plane.
Plane planeFromLine(const ScreenLine &screenLine,
                    const CoordinateSystem &cameraSystem) {
  auto [p1, p2] = pointsOnLine(screenLine);
  Vector d1 = unprojectPoint(cameraSystem, p1) - cameraSystem.origin;
  Vector d2 = unprojectPoint(cameraSystem, p2) - cameraSystem.origin;
  Vector perp = crossP(d1, d2);
  Vector normal = perp * (1. / sqrt(perp * perp));
  double d = normal * cameraSystem.origin;
  return {normal, d};
}

/// Returns the squared distance of the point to the plane of the line.
double loss(const std::pair<ScreenLine, Vector> &constraint,
            const Pose &currentEstimate) {
  CoordinateSystem cameraSystem = getCameraSystem(currentEstimate);
  Plane plane = planeFromLine(constraint.first, cameraSystem);
  double dist = constraint.second * plane.normal - plane.d;
  return dist * dist;
}

/// Matches a ScreenLine to a line on the board by taking the line on the board
/// where the points on that boardline which are on the screen have the smallest
/// squared distance to the line. Returns 3D points on that line which are on
/// the screen.
std::vector<Vector> matchBoardLine(const ScreenLine &screenLine,
                                   const Pose &posePreviousFrame,
                                   std::array<Line, 4> candidates,
                                   std::string screenLineName) {
  CoordinateSystem cameraSystemPreviousFrame =
      getCameraSystem(posePreviousFrame);
  std::vector<Vector> points;
  double bestAv = 1e9;
  Line bLine = Line::BORDER_OUT_1;
  for (long unsigned int i = 0; i < candidates.size(); ++i) {
    auto [s, e] = getStartEndPoints(candidates[i]);
    auto se = e - s;
    double sum = 0;
    std::vector<Vector> tpoints;
    for (double t = 0; t < 1. + 1e-5; t += 5. / std::sqrt(se * se)) {
      Vector v = s * (1. - t) + e * t;
      if (auto projected = projectPoint(cameraSystemPreviousFrame, v)) {
        if (projected->x >= 0 && projected->x < WIDTH && projected->y >= 0 &&
            projected->y < HEIGHT) {
          sum += std::pow(projected->x * -sin(screenLine.angle) +
                              projected->y * cos(screenLine.angle) -
                              screenLine.distanceToOrigin,
                          2);
          tpoints.push_back(v);
        }
      }
    }
    if (tpoints.size() < 3)
      continue;
    double average = sum / double(tpoints.size());
    if (average < bestAv) {
      bestAv = average;
      points = tpoints;
      bLine = candidates[i];
    }
  }
  // TODO: tweak this number
  if (bestAv < 1e4) {
    std::cout << "Matched " << screenLineName << " to " << int(bLine)
              << " with average = " << bestAv << std::endl;
    return points;
  } else {
    std::cout << "Wasn't able to find a line which makes sufficient amount of "
                 "sense for "
              << screenLineName << ". Best would have been " << int(bLine)
              << " with average = " << bestAv << std::endl;
    return {};
  }
}

/// Returns a vector of constraints for the optimization process. Each
/// constraint is of the form that a 3D point (and endpoint of the matched line)
/// needs to be close to the plane of a ScreenLine. Thus we represent them as
/// pairs of ScreenLine and 3D Points.
std::vector<std::pair<ScreenLine, Vector>>
getConstraints(const ScreenLineSet &screenLines,
               const Pose &posePreviousFrame) {
  std::vector<std::pair<ScreenLine, Vector>> constraints;
  auto matchAddConstraints = [&](const std::optional<ScreenLine> &screenLineOpt,
                                 std::array<Line, 4> candidates,
                                 std::string screenLineName) {
    if (!screenLineOpt.has_value())
      return;
    ScreenLine screenLine = *screenLineOpt;
    for (auto p : matchBoardLine(screenLine, posePreviousFrame, candidates,
                                 screenLineName)) {
      constraints.push_back({screenLine, p});
    }
  };
  matchAddConstraints(screenLines.blue, blueLines, "blue");
  matchAddConstraints(screenLines.orange, orangeLines, "orange");
  matchAddConstraints(screenLines.left, outerLines, "left");
  matchAddConstraints(screenLines.back, outerLines, "back");
  matchAddConstraints(screenLines.right, innerLines, "right");
  return constraints;
}

/// Returns the gradient of the cost function on all pose parameters (the return
/// type is Pose but in reality only a gradient on the pose parameters are
/// returned, it's just the exact same fields so we also use pose) at the
/// current estimate for the pose. The cost function we use is the squared
/// distance of the distance of the point to the plane.
Pose getGrad(const std::pair<ScreenLine, Vector> &constraint,
             const Pose &currentEstimate) {
  // It's crazy that this works. I thought the formula would be way more ugly.
  // It even seems to work with camera twist. No idea why, I though it'd then
  // require some rotation of tangent but apparently no.
  CoordinateSystem cameraSystem = getCameraSystem(currentEstimate);
  double depth = constraint.second * cameraSystem.z;
  Plane plane = planeFromLine(constraint.first, cameraSystem);
  double dist = constraint.second * plane.normal - plane.d;
  Vector closePoint = constraint.second - plane.normal * dist;
  Vector closePointCamera = closePoint - cameraSystem.origin;
  Vector diff = closePoint - constraint.second;
  Vector tangent = {-closePointCamera.y, closePointCamera.x, 0};
  double thetaGrad = tangent * diff;
  // Make gradients from constraints with high depth smaller since there
  // distance corresponds to smaller distance on screen.
  double fac = 1. / std::max(10., depth);
  return {diff.x * fac, diff.y * fac, thetaGrad * 0.001 * fac};
}

void printPose(const Pose &pose) {
  std::cout << "Pose(x=" << pose.x << ", y=" << pose.y
            << ", theta=" << pose.theta << ")" << std::endl;
}

/// Uses gradient descent to find the pose for which the given constraints are
/// satisfied as well as possible.
std::optional<Pose> optimizePose(const ScreenLineSet &screenLines,
                                 const Pose &posePreviousFrame, Pose &debug) {
  auto constraints = getConstraints(screenLines, posePreviousFrame);
  std::cout << "constaints.size() = " << constraints.size() << std::endl;
  Pose pose = posePreviousFrame;
  printPose(pose);
  const float learningRate = 3.;
  for (int epoch = 0; epoch < 1000; ++epoch) {
    Pose adjustment{0, 0, 0};
    for (auto constraint : constraints) {
      adjustment = adjustment + getGrad(constraint, pose);
    }
    pose = pose + adjustment * (-learningRate / float(constraints.size()));
    if (true) {
      std::cout << "theta: " << pose.theta << ", x: " << pose.x
                << ", y: " << pose.y << std::endl;
    }
  }
  double totalLoss = 0;
  for (auto constraint : constraints) {
    totalLoss += loss(constraint, pose);
  }
  std::cout << "final pose loss: " << totalLoss << std::endl;
  debug = pose;
  // TODO: tweak this number.
  if (totalLoss < 1000.) {
    return {pose};
  } else {
    std::cout
        << "No pose which makes a sufficient amount of sense could be found"
        << std::endl;
    return {};
  }
}

void printCoordinateSystem(const CoordinateSystem &cs) {
  std::cout << "Coordinate System:\n";
  printVector("x-axis", cs.x);
  printVector("y-axis", cs.y);
  printVector("z-axis", cs.z);
  printVector("origin", cs.origin);
}

#ifdef CAM_TESTING
int main() {
  Pose pose{200, 50, 0.4 + M_PI};
  auto cs = getCameraSystem(pose);
  printCoordinateSystem(cs);
  auto p = unprojectPoint(cs, {750, 300});
  printVector("unproj", p);
  auto projected = projectPoint(cs, p);
  if (projected.has_value())
    std::cerr << "Screen position: x = " << projected->x
              << ", y = " << projected->y << std::endl;
  ScreenLineSet screenLines{{}, {}, {}, ScreenLine{0.1, 200}, {}};
  ScreenLine screenLine = *screenLines.back;
  auto optPose = *optimizePose(screenLines, pose);
  auto plane = planeFromLine(screenLine, getCameraSystem(optPose));
  // printPlaneToBlenderScript(plane);
  std::cout << "d = " << plane.d << std::endl;
  printVector("normal", plane.normal);

  initializeCamera();

  drawProjectedLines(lastFrame, pose);
  saveFrame(lastFrame);

  cleanCamera();

  return 0;
}
#endif

#ifdef GRAD_TESTING
int main() {
  Pose pose{50, 150, M_PI};
  ScreenLine screenLine{M_PI_2, 200};
  Vector p = getStartEndPoints(Line::BORDER_OUT_1).second;
  std::pair<ScreenLine, Vector> constraint{screenLine, p};
  auto grad = getGrad(constraint, pose);
  const double eps = 1e-8;
  double preLoss = loss(constraint, pose);
  double postXLoss = loss(constraint, pose + Pose{eps, 0, 0});
  double postYLoss = loss(constraint, pose + Pose{0, eps, 0});
  double postTLoss = loss(constraint, pose + Pose{0, 0, eps});
  std::cerr << "XGrad: " << grad.x * 2
            << ", XNumGrad: " << (postXLoss - preLoss) / eps << std::endl;
  std::cerr << "YGrad: " << grad.y * 2
            << ", YNumGrad: " << (postYLoss - preLoss) / eps << std::endl;
  std::cerr << "TGrad: " << grad.theta / 0.001 * 2
            << ", TNumGrad: " << (postTLoss - preLoss) / eps << std::endl;
}
#endif
