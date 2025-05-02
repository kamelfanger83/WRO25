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
  Pose lastArduinoPose = {0, 0, 0};
  ControllerState controllerState{0};

  long long lastTimeStamp = -1;

  double blindError = 0.;
  Commands commands;

  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    auto arduinoPose = processArduinoResponse();
    if (arduinoPose.has_value()) {
      auto diffPose = *arduinoPose / lastArduinoPose;
      blindError +=
          std::sqrt(diffPose.x * diffPose.x + diffPose.y * diffPose.y) +
          100 * std::abs(diffPose.theta);
      pose = pose * diffPose;
      lastArduinoPose = *arduinoPose;
    }

    if (blindError > 50.) {
      blindError = 0.;
      commands.speed = 0;
      sendCommands(commands);

      std::this_thread::sleep_for(std::chrono::milliseconds(50));

      lastTimeStamp = lastFrame.timestamp;
      queueCapture();
      while (lastTimeStamp == lastFrame.timestamp) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
      std::cout << "Captured a frame at: " << lastFrame.timestamp << std::endl;

      std::cout << "Pre visual pose:\n";
      printPose(pose);
      auto screenLines = findLines(lastFrame, pose);
      drawScreenLineSet(lastFrame, screenLines);
      auto poset = optimizePose(screenLines, pose);
      if (!poset.has_value()) {
        std::cout << "AW HELL NAW" << std::endl;
        break;
      }
      pose = *poset;

      drawProjectedLines(lastFrame, pose);
      saveFrame(lastFrame);

      std::cout << "Post visual pose:\n";
      printPose(pose);
    }

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

    commands = getCommands(pose, waypoints.front(), controllerState);
    // std::cout << "Aiming for commands: ang=" << commands.angle
    //          << ", speed=" << commands.speed << std::endl;
    sendCommands(commands);
  }

  cleanCamera();

  return 0;
}
