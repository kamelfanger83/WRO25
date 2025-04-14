#include <cmath>
#include <functional>
#include <iostream>
#include <optional>
#include <queue>

#include "camera/camera.h"
#include "camera/find_line.cpp"
#include "geo/camera.cpp"
#include "getCommands/getCommands.cpp"
#include "serial.cpp"
#include "structs.h"

struct Mode {
  std::function<std::optional<std::pair<std::queue<Waypoint>, Mode>>(
      const Frame &, const Pose &)>
      plan;
};

std::optional<std::pair<std::queue<Waypoint>, Mode>>
endMode(const Frame &frame, const Pose &position) {
  return {};
}

std::optional<std::pair<std::queue<Waypoint>, Mode>>
startMode(const Frame &frame, const Pose &position) {
  std::queue<Waypoint> waypoints;
  Waypoint startPoint;
  startPoint.x = position.x;
  startPoint.y = position.y;

  waypoints.push({50, 250});
  waypoints.push({250, 250});
  waypoints.push({250, 50});
  waypoints.push({50, 50});
  waypoints.push(startPoint);

  return {std::make_pair(waypoints, Mode{endMode})};
}

std::optional<std::pair<std::queue<Waypoint>, Mode>>
endSegmentMode(const Frame &frame, const Pose &position) {
  return {};
}

int main() {
  initializeCamera();
  initializeSerial();

  std::queue<Waypoint> waypoints;
  Mode nextMode = {startMode};
  Pose pose = {50, 150, M_PI_2};
  ControllerState controllerState{0};

  while (true) {
    auto poset = optimizePose(findLines(lastFrame), pose);
    if (!poset.has_value()) {
      std::cout << "AW HELL NAW" << std::endl;
      break;
    }
    pose = *poset;
    if (waypoints.front().reached(pose)) {
      waypoints.pop();
    }
    if (waypoints.empty()) {
      auto result = nextMode.plan(lastFrame, pose);
      if (!result.has_value()) {
        break;
      }
      waypoints = result->first;
      nextMode = result->second;
    }
    Commands commands = getCommands(pose, waypoints.front(), controllerState);
    sendCommands(commands);
  }
  return 0;
}
