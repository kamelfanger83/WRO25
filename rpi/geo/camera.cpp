#include <cmath>
#include <iomanip>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "../structs.h"
#include "coordinates.cpp"
#include "utils.cpp"
// We only need this for width and height. Not ideal.
#include "../camera/camera.h"

/// The height of the camera origin above the mat in cm.
#define CAM_HEIGHT 0
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
  double cameraX = camVec * cameraSystem.x;
  double cameraY = camVec * cameraSystem.y;
  double horizontalRange = 2. * std::tan(HORIZONTAL_FOV / 2);
  double verticalRange = horizontalRange / WIDTH * HEIGHT;
  double x = cameraX / horizontalRange * WIDTH + double(WIDTH) / 2.;
  double y = cameraY / verticalRange * HEIGHT + double(HEIGHT) / 2.;
  return {{x, y}};
}

// TODO: implement these functions
/// Returns two arbitrary points on a given ScreenLine.
// Get one of the points by multiplying the vector perpendicular to the line in
// the correct direction with distanceToOrigin. Then get the second one for
// example by just adding the direction vector of the line once.
std::pair<ScreenPosition, ScreenPosition>
pointsOnLine(const ScreenLine &screenLine);
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
                    const CoordinateSystem &cameraSystem);
/// Matches a ScreenLine to a line on the board by taking the line on the board
/// where the endpoints have the smallest distance to the plane of the
/// ScreenLine. Also discards candidates which are very far from being visible
/// on the screen.
Line matchBoardLine(const ScreenLine &screenLine,
                    const CoordinateSystem &cameraSystemPreviousFrame,
                    Line candidates[4]);
/// Returns a vector of constraints for the optimization process. Each
/// constraint is of the form that a 3D point (and endpoint of the matched line)
/// needs to be close to the plane of a ScreenLine. Thus we represent them as
/// pairs of ScreenLine and 3D Points.
std::vector<std::pair<ScreenLine, Vector>>
getConstraints(const ScreenLineSet &screenLines,
               const CoordinateSystem &cameraSystemPreviousFrame);

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
  return {diff.x, diff.y, tangent * diff};
}

// Can be used for testing and as a break condition.
double loss(const std::pair<ScreenLine, Vector> &constraint,
            const Pose &currentEstimate) {

  CoordinateSystem cameraSystem = getCameraSystem(currentEstimate);
  Plane plane = planeFromLine(constraint.first, cameraSystem);
  double dist = constraint.second * plane.normal - plane.d;
  return dist * dist;
}

/// Uses gradient descent to find the pose for which the given constraints are
/// satisfied as well as possible.
Pose optimizePose(const std::vector<std::pair<ScreenLine, Vector>> &constraints,
                  const Pose &startPoint);

// Temporary testing functions
void printVector(const std::string &name, const Vector &v) {
  std::cout << std::fixed << std::setprecision(3);
  std::cout << "  " << name << ": (" << v.x << ", " << v.y << ", " << v.z
            << ")\n";
}

void printCoordinateSystem(const CoordinateSystem &cs) {
  std::cout << "Coordinate System:\n";
  printVector("x-axis", cs.x);
  printVector("y-axis", cs.y);
  printVector("z-axis", cs.z);
  printVector("origin", cs.origin);
}

int main() {
  Pose pose{0, 0, 0.4};
  auto cs = getCameraSystem(pose);
  printCoordinateSystem(cs);
  auto p = unprojectPoint(cs, {750, 300});
  printVector("unproj", p);
  auto projected = projectPoint(cs, p);
  if (projected.has_value())
    std::cerr << "Screen position: x = " << projected->x
              << ", y = " << projected->y << std::endl;
}
