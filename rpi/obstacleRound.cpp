#include <chrono>
#include <cmath>
#include <functional>
#include <iostream>
#include <optional>
#include <queue>
#include <thread>

#include "camera/camera.h"
#include "camera/find_color.cpp"
#include "camera/find_line.cpp"
#include "camera/obstacles.cpp"
#include "driver.cpp"
#include "geo/camera.cpp"
#include "geo/coordinates.cpp"
#include "geo/coordinatesTrafficlights.cpp"
#include "getCommands/getCommands.cpp"
#include "park.cpp"
#include "serial.cpp"
#include "structs.h"

std::optional<char> checkTrafficLightInSegment(Frame &frame, Segment s,
                                               const Pose &position,
                                               TrafficLight tl) {
  auto coords = getTrafficLightCoordinates(tl);
  for (Vector &corner : coords) {
    corner = vectorInSegment(s, corner);
  }
  auto ret = checkTrafficLight(frame, getCameraSystem(position), coords);

  saveFrame(frame);

  return ret;
}
/// have traffic light check function in obstacles.cpp signature:
/*std::optional<char>
checkTrafficLight(Frame &frame, const CoordinateSystem &cameraSystem,
  const std::vector<Vector> &unprojectedPoints)*/

bool flipped = false;

std::optional<std::pair<std::queue<Waypoint>, Mode>>
endMode(Frame &frame, const Pose &position) {
  return {};
}

std::optional<std::pair<std::queue<Waypoint>, Mode>>
modeFromEndRight(Frame &frame, const Pose &position);

std::optional<std::pair<std::queue<Waypoint>, Mode>>
modeFromEndLeft(Frame &frame, const Pose &position);

std::optional<std::pair<std::queue<Waypoint>, Mode>>
startMode(Frame &frame, const Pose &position) {

  std::queue<Waypoint> waypoints;

  /// three case, no traffic light in middle, red light, green light;

  auto trafficLight = checkTrafficLightInSegment(
      frame, Segment::SEGMENT_1, position, TrafficLight::TRAFFICLIGHT_4);

  if (!trafficLight.has_value()) {
    waypoints.push({45, 150, true});
    return {{waypoints, Mode{endMode}}};
    // return {{waypoints, Mode{modeFromMiddle}}};
  } else if (trafficLight.value() == 'r') {
    waypoints.push({48, 125, false});
    waypoints.push({68, 130, false});
    waypoints.push({75, 160, true});
    waypoints.push({80, 190, true});
    return {{waypoints, Mode{modeFromEndRight}}};
  } else {
    waypoints.push({35, 150, true});
    waypoints.push({27, 190, true});
    waypoints.push({45, 228, true});
    return {{waypoints, Mode{modeFromEndLeft}}};
    // return {{waypoints, Mode{modeFromEndLeft}}};
  }
}

/// mode from the middle,
/*std::optional<std::pair<std::queue<Waypoint>, Mode>>
modeFromMiddle(Frame &frame, const Pose &position){

  /// cases: on position 5 or on position 6. left or right.
 Segment curr = inSegment(position);
 std::queue<Waypoint> waypoints;

 if(!(checkTrafficLightInSegment(frame, curr, position,
TrafficLight::TRAFFICLIGHT_5).has_value() &&
    !(checkTrafficLightInSegment(frame, curr, position,
TrafficLight::TRAFFICLIGHT_6)).has_value())){

  waypoints.push(wayPointInSegment(curr, {20, 200}));
  waypoints.push(wayPointInSegment(curr, {30, 228}));

  return {{waypoints, Mode{modeFromEndLeft}}};
} */

/// mode from front right,
std::optional<std::pair<std::queue<Waypoint>, Mode>>
modeFromEndRight(Frame &frame, const Pose &position) {

  Segment current = nextSegment(inSegment(position));

  std::queue<Waypoint> waypoints;
  auto first = checkTrafficLightInSegment(frame, current, position,
                                          TrafficLight::TRAFFICLIGHT_1);
  auto second = checkTrafficLightInSegment(frame, current, position,
                                           TrafficLight::TRAFFICLIGHT_2);

  assert(!(first.has_value() && second.has_value()));

  if (!(first.has_value() || second.has_value())) {
    waypoints.push(wayPointInSegment(current, {80, 100, true}));
    // waypoints.push(wayPointInSegment(current, {50, 95}));
    // waypoints.push(wayPointInSegment(current, {50, 100}));
    return {{waypoints, Mode{endMode}}};
    // return {{waypoints, Mode{modeFromFirstLine}}};

  } else if ((first.has_value() && *first == 'r') ||
             (second.has_value() && *second == 'r')) {
    waypoints.push(wayPointInSegment(current, {80, 100, true}));
    waypoints.push(wayPointInSegment(current, {50, 140, false}));
    waypoints.push(wayPointInSegment(current, {50, 150, true}));
    return {{waypoints, Mode{endMode}}};
    // return {{waypoints, mode{modefrommiddle}}};

  } else if ((first.has_value() && *first == 'g') ||
             (second.has_value() && *second == 'g')) {
    waypoints.push(wayPointInSegment(current, {65, 65, false}));
    waypoints.push(wayPointInSegment(current, {20, 100, true}));
    waypoints.push(wayPointInSegment(current, {40, 125, false}));
    waypoints.push(wayPointInSegment(current, {40, 150, true}));
    return {{waypoints, Mode{endMode}}};
    // return {{waypoints, mode{modefrommiddle}}};
  }
  return {};
}

std::optional<std::pair<std::queue<Waypoint>, Mode>>
modeFromEndLeft(Frame &frame, const Pose &position) {

  Segment current = nextSegment(inSegment(position));

  std::queue<Waypoint> waypoints;
  auto first = checkTrafficLightInSegment(frame, current, position,
                                          TrafficLight::TRAFFICLIGHT_1);
  auto second = checkTrafficLightInSegment(frame, current, position,
                                           TrafficLight::TRAFFICLIGHT_2);

  assert(!(first.has_value() && second.has_value()));

  if (!(first.has_value() || second.has_value())) {
    waypoints.push(wayPointInSegment(current, {80, 100, true}));
    // waypoints.push(wayPointInSegment(current, {50, 95}));
    // waypoints.push(wayPointInSegment(current, {50, 100}));
    return {{waypoints, Mode{endMode}}};
    // return {{waypoints, Mode{modeFromFirstLine}}};

  } else if ((first.has_value() && *first == 'r') ||
             (second.has_value() && *second == 'r')) {
    waypoints.push(wayPointInSegment(current, {80, 100, true}));
    waypoints.push(wayPointInSegment(current, {50, 140, false}));
    waypoints.push(wayPointInSegment(current, {50, 150, true}));
    return {{waypoints, Mode{endMode}}};
    // return {{waypoints, mode{modefrommiddle}}};

  } else if ((first.has_value() && *first == 'g') ||
             (second.has_value() && *second == 'g')) {
    waypoints.push(wayPointInSegment(current, {20, 100, true}));
    waypoints.push(wayPointInSegment(current, {40, 125, false}));
    waypoints.push(wayPointInSegment(current, {40, 150, true}));
    return {{waypoints, Mode{endMode}}};
    // return {{waypoints, mode{modefrommiddle}}};
  }
  return {};
}

int main() {
  initializeCamera();
  initializeSerial();

  processArduinoResponse();

  unpark();

  run({startMode}, /* {21, 123, 0.8} */ {8.6, 111, M_PI_2}, false);

  cleanCamera();

  return 0;
}
