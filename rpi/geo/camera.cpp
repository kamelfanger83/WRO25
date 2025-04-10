#include <cmath>
#include <iomanip>
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "../structs.h"
#include "../utils.cpp"
#include "coordinates.cpp"
#include "utils.cpp"
// We only need this for width and height. Not ideal.
#include "../camera/camera.h"

/// The height of the camera origin above the mat in cm.
#define CAM_HEIGHT 20
/// The angle by which the camera is tilted down in radians.
#define TILT_ANG M_PI / 13
/// The horizontal FOV angle in radians.
#define HORIZONTAL_FOV M_PI / 2

/// Describes another coordinate system relative to the standard coordinate
/// system. x, y, z form an orthonormal basis and point in the positive x, y, z
///     direction of the coordinate system.
/// origin is the standard location of the point (0, 0, 0) in the coordinate
/// system.
struct CoordinateSystem {
  Vector x, y, z, origin;
};

CoordinateSystem getCameraSystem(const Pose &pose) {
  Vector x{std::cos(pose.theta - M_PI_2f64), std::sin(pose.theta - M_PI_2f64),
           0};
  Vector y{-std::sin(TILT_ANG) * std::cos(pose.theta),
           -std::sin(TILT_ANG) * std::sin(pose.theta), -std::cos(TILT_ANG)};
  Vector z{std::cos(TILT_ANG) * std::cos(pose.theta),
           std::cos(TILT_ANG) * std::sin(pose.theta), -std::sin(TILT_ANG)};
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

// Matches a ScreenLine to a line on the board by taking the line on the board
/// where the endpoints have the smallest distance to the plane of the
/// ScreenLine. Also discards candidates which are very far from being visible
/// on the screen.
std::optional<Line> matchBoardLine(const ScreenLine &screenLine,
                                   const Pose &posePreviousFrame,
                                   Line candidates[4]) {
  CoordinateSystem cameraSystemPreviousFrame =
      getCameraSystem(posePreviousFrame);
  Line bestCandidate = Line::INVALID;
  double bestLoss = 1e9;
  for (int i = 0; i < 4; ++i) {
    auto [s, e] = getStartEndPoints(candidates[i]);
    bool inRange = false;
    for (double t = 0; t < 1. + 1e-5 && !inRange; t += 0.05) {
      Vector v = s * (1. - t) + e * t;
      if (auto projected = projectPoint(cameraSystemPreviousFrame, v)) {
        inRange |= projected->x >= 0 && projected->x < WIDTH &&
                   projected->y >= 0 && projected->y < HEIGHT;
      }
    }
    if (!inRange)
      continue;
    double candidateLoss = loss({screenLine, s}, posePreviousFrame) +
                           loss({screenLine, e}, posePreviousFrame);
    if (candidateLoss < bestLoss) {
      bestLoss = candidateLoss;
      bestCandidate = candidates[i];
    }
  }
  // TODO: tweak this number
  if (bestLoss < /*200. */ 1e9)
    return {bestCandidate};
  else
    return {};
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
                                 Line candidates[4]) {
    if (!screenLineOpt.has_value())
      return;
    ScreenLine screenLine = *screenLineOpt;
    if (auto line = matchBoardLine(screenLine, posePreviousFrame, candidates)) {
      auto [s, e] = getStartEndPoints(*line);
      constraints.push_back({screenLine, s});
      constraints.push_back({screenLine, e});
    }
  };
  matchAddConstraints(screenLines.blue, blueLines);
  matchAddConstraints(screenLines.orange, orangeLines);
  matchAddConstraints(screenLines.outer, outerLines);
  matchAddConstraints(screenLines.hind, outerLines);
  return constraints;
}

/// Returns the gradient of the cost function on all pose parameters (the return
/// type is Pose but in reality only a gradient on the pose parameters are
/// returned, it's just the exact same fields so we also use pose) at the
/// current estimate for the pose. The cost function we use is the squared
/// distance of the distance of the point to the plane.
Pose getGrad(const std::pair<ScreenLine, Vector> &constraint,
             const Pose &currentEstimate) {
  // I am only 60% confident that this is correct. It seems a bit too nice to be
  // correct.
  CoordinateSystem cameraSystem = getCameraSystem(currentEstimate);
  Plane plane = planeFromLine(constraint.first, cameraSystem);
  double dist = constraint.second * plane.normal - plane.d;
  Vector closePoint = constraint.second - plane.normal * dist;
  Vector closePointCamera = closePoint - cameraSystem.origin;
  Vector diff = closePoint - constraint.second;
  Vector tangent = {-closePointCamera.y, closePointCamera.x, 0};
  double thetaGrad = tangent * diff;
  return {diff.x, diff.y, thetaGrad * 0.001};
}

void printPose(const Pose &pose) {
  std::cout << "Pose(x=" << pose.x << ", y=" << pose.y
            << ", theta=" << pose.theta << ")" << std::endl;
}

/// Uses gradient descent to find the pose for which the given constraints are
/// satisfied as well as possible.
std::optional<Pose> optimizePose(const ScreenLineSet &screenLines,
                                 const Pose &posePreviousFrame) {
  auto constraints = getConstraints(screenLines, posePreviousFrame);
  Pose pose = posePreviousFrame;
  printPose(pose);
  const float learningRate = 0.01;
  for (int epoch = 0; epoch < 1000; ++epoch) {
    Pose adjustment{0, 0, 0};
    int i = 0;
    for (auto constraint : constraints) {
      adjustment = adjustment + getGrad(constraint, pose);
    }
    pose = pose + adjustment * -learningRate;
    // printPose(pose);
  }
  double totalLoss = 0;
  for (auto constraint : constraints) {
    totalLoss += loss(constraint, pose);
  }
  // TODO: tweak this number.
  if (totalLoss < 200.) {
    return {pose};
  } else {
    std::cerr
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
  ScreenLineSet screenLines{{}, {}, {}, ScreenLine{0.1, 200}};
  ScreenLine screenLine = *screenLines.hind;
  auto match = matchBoardLine(screenLine, pose, outerLines);
  if (match.has_value())
    std::cout << "We found a match: " << *match << std::endl;
  else
    std::cout << "No match was found" << std::endl;
  auto optPose = *optimizePose(screenLines, pose);
  auto plane = planeFromLine(screenLine, getCameraSystem(optPose));
  // printPlaneToBlenderScript(plane);
  std::cout << "d = " << plane.d << std::endl;
  printVector("normal", plane.normal);
}
