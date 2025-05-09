#include <cmath>
#include <optional>
#include <queue>

#include "camera/camera.h"
#include "camera/find_color.cpp"
#include "camera/obstacles.cpp"
#include "driver.cpp"
#include "geo/camera.cpp"
#include "geo/coordinates.cpp"
#include "geo/coordinatesTrafficlights.cpp"
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
modeFromMiddle(Frame &frame, const Pose &position);

std::optional<std::pair<std::queue<Waypoint>, Mode>>
startMode(Frame &frame, const Pose &position);

std::optional<std::pair<std::queue<Waypoint>, Mode>>
startMode(Frame &frame, const Pose &position) {

  std::queue<Waypoint> waypoints;

  /// three case, no traffic light in middle, red light, green light;

  auto trafficLight = checkTrafficLightInSegment(
      frame, Segment::SEGMENT_1, position, TrafficLight::TRAFFICLIGHT_4);

  if (!trafficLight.has_value()) {
    waypoints.push({45, 150, true});
    return {{waypoints, Mode{modeFromMiddle}}};
  } else if (trafficLight.value() == 'r') {
    waypoints.push({48, 125, false});
    waypoints.push({68, 130, false});
    waypoints.push({75, 160, true});
    waypoints.push({80, 190, true});
    return {{waypoints, Mode{modeFromEndRight}}};
  } else {
    waypoints.push({35, 150, true});
    waypoints.push({30, 180, true});
    waypoints.push({50, 220, true});
    return {{waypoints, Mode{modeFromEndLeft}}};
    // return {{waypoints, Mode{modeFromEndLeft}}};
  }
}

/// mode from the middle,
std::optional<std::pair<std::queue<Waypoint>, Mode>>
modeFromMiddle(Frame &frame, const Pose &position) {

  /// cases: on position 5 or on position 6. left or right.
  Segment curr = inSegment(position);
  std::queue<Waypoint> waypoints;

  auto lightLeft = checkTrafficLightInSegment(frame, curr, position,
                                              TrafficLight::TRAFFICLIGHT_5);
  auto lightRight = checkTrafficLightInSegment(frame, curr, position,
                                               TrafficLight::TRAFFICLIGHT_6);

  if (!(lightLeft.has_value()) && !(lightRight.has_value())) {

    waypoints.push(wayPointInSegment(curr, {30, 190, true}));
    waypoints.push(wayPointInSegment(curr, {50, 230, true}));

    return {{waypoints, Mode{modeFromEndLeft}}};
  }
  if ((lightLeft.has_value() && *lightLeft == 'r') ||
      (lightRight.has_value() && *lightRight == 'r')) {
    waypoints.push(wayPointInSegment(curr, {60, 170, false}));
    waypoints.push(wayPointInSegment(curr, {80, 200, true}));
    return {{waypoints, Mode{modeFromEndRight}}};
  }

  if ((lightLeft.has_value() && *lightLeft == 'g') || // links durch
      (lightRight.has_value() && *lightRight == 'g')) {
    waypoints.push(wayPointInSegment(curr, {25, 175, true}));
    waypoints.push(wayPointInSegment(curr, {50, 220, true}));
    return {{waypoints, Mode{modeFromEndLeft}}};
  }
  return {};
}

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
    waypoints.push(wayPointInSegment(current, {80, 80}));
    waypoints.push(wayPointInSegment(current, {55, 100}));
    // return {{waypoints, Mode{endMode}}};
    return {{waypoints, Mode{modeFromFirstLine}}};

  } else if ((first.has_value() && *first == 'r') || // rechts durch
             (second.has_value() && *second == 'r')) {
    // waypoints.push(wayPointInSegment(current, {100, 100, true}));
    waypoints.push(wayPointInSegment(current, {80, 100, true}));
    waypoints.push(wayPointInSegment(current, {60, 130, false}));

    // waypoints.push(wayPointInSegment(current, {65, 110, false}));
    waypoints.push(wayPointInSegment(current, {60, 150, true}));
    // waypoints.push(wayPointInSegment(current, {70, 125, true}));
    //  waypoints.push(wayPointInSegment(current, {70, 140, true}));
    return {{waypoints, Mode{modeFromMiddle}}};

  } else if ((first.has_value() && *first == 'g') || // links durch
             (second.has_value() && *second == 'g')) {
    waypoints.push(wayPointInSegment(current, {65, 65, false}));
    waypoints.push(wayPointInSegment(current, {20, 70, true}));

    waypoints.push(wayPointInSegment(current, {20, 100, true}); // cross first line
    waypoints.push(wayPointInSegment(current, {35, 110, false}));
    waypoints.push(wayPointInSegment(current, {50, 140, true}));

    return {{waypoints, Mode{modeFromMiddle}}};
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
    waypoints.push(wayPointInSegment(current, {50, 100, true}));
    // waypoints.push(wayPointInSegment(current, {50, 95}));
    // waypoints.push(wayPointInSegment(current, {50, 100}));
    // return {{waypoints, Mode{endMode}}};
    return {{waypoints, Mode{modeFromFirstLine}}};

  } else if ((first.has_value() && *first == 'r') ||
             (second.has_value() && *second == 'r')) {
    waypoints.push(wayPointInSegment(current, {82, 100, false}));
    waypoints.push(wayPointInSegment(current, {65, 125, true}));
    // waypoints.push(wayPointInSegment(current, {65, 140, true}));
    return {{waypoints, Mode{modeFromMiddle}}};

  } else if ((first.has_value() && *first == 'g') ||
             (second.has_value() && *second == 'g')) {
    waypoints.push(wayPointInSegment(current, {45, 70, false}));
    waypoints.push(wayPointInSegment(current, {15, 100, true}));
    waypoints.push(wayPointInSegment(current, {40, 140, true}));
    return {{waypoints, Mode{modeFromMiddle}}};
  }
  return {};
}

std::optional<std::pair<std::queue<Waypoint>, Mode>>
modeFromFirstLine(Frame &frame, const Pose &position) {
  Segment curr = nextSegment(inSegment(position));
  std::queue<Waypoint> waypoints = {};

  auto third = checkTrafficLightInSegment(frame, curr, position,
                                          TrafficLight::TRAFFICLIGHT_3);
  auto fourth = checkTrafficLightInSegment(frame, curr, position,
                                           TrafficLight::TRAFFICLIGHT_4);

  assert(!(third.has_value() && fourth.has_value()));

  auto goToEndLeft = [&waypoints, curr]() {
    waypoints.push(wayPointInSegment(curr, {20, 200, true}));
    waypoints.push(wayPointInSegment(curr, {30, 228, true}));
  };

  if ((third.has_value() && *third == 'r') ||
      (fourth.has_value() && *fourth == 'r')) {
    waypoints.push(wayPointInSegment(curr, {75, 145, true}));
    waypoints.push(wayPointInSegment(curr, {80, 150, true})); // right of the 4. tl.
    waypoints.push(wayPointInSegment(curr, {80, 200, true}));
    return {{waypoints, Mode{modeFromEndRight}}};
  } else if ((third.has_value() && *third == 'g') ||
             (fourth.has_value() && *fourth == 'g')) {
    waypoints.push(wayPointInSegment(curr, {25, 140, true}));
    waypoints.push(wayPointInSegment(curr, {20, 150, true})); // left of the 3. tl.
    goToEndLeft();
    return {{waypoints, Mode{modeFromEndLeft}}};
  } else {
    assert(!(third.has_value()) && !(fourth.has_value()));
    waypoints.push(wayPointInSegment(curr, {50, 145, true}));
    return {{waypoints, Mode{modeFromMiddle}}};
  }
}

int main() {
  initializeCamera();
  initializeSerial();

  waitForGo();

  processArduinoResponse();

  unpark();

  run({startMode}, /* {21, 123, 0.8} */ {8.6, 111, M_PI_2}, false);

  cleanCamera();

  return 0;
}
