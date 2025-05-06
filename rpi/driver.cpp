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
#include "utils.cpp"

struct Mode {
  std::function<std::optional<std::pair<std::queue<Waypoint>, Mode>>(
      Frame &, const Pose &)>
      plan;
};

int run(const Mode &startMode, const Pose &startPose, bool ignoreInner) {
  std::queue<Waypoint> waypoints;
  Mode nextMode = startMode;
  Pose pose = startPose;
  Pose lastArduinoPose = {0, 0, 0};
  ControllerState controllerState{0};

  double blindError = 1e6;
  Commands commands{84, 0};

  auto positionVisual = [&]() -> void {
    blindError = 0.;
    commands.speed = 0;
    sendCommands(commands);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    captureFrameBlocking();
    std::cout << "Captured a frame at: " << lastFrame.timestamp << std::endl;

    saveFrame(lastFrame);
    lastFrame.timestamp += 1;

    std::cout << "Pre visual pose:\n";
    printPose(pose);
    auto screenLines = findLines(lastFrame, pose);
    drawScreenLineSet(lastFrame, screenLines);
    if (ignoreInner)
      screenLines.right = {};

    drawProjectedLines(lastFrame, pose, {205, 255, 166});
    Pose debug = pose;
    auto poset = optimizePose(screenLines, pose, debug);
    drawProjectedLines(lastFrame, debug, {43, 255, 255});

    saveFrame(lastFrame);
    if (!poset.has_value()) {
      std::cout << "AW HELL NAW" << std::endl;
      exit(-1);
    }
    pose = poset->first;

    std::cout << "Post visual pose:\n";
    printPose(pose);
  };

  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    auto arduinoPose = processArduinoResponse();
    if (arduinoPose.has_value()) {
      auto diffPose = *arduinoPose / lastArduinoPose;
      blindError +=
          std::sqrt(diffPose.x * diffPose.x + diffPose.y * diffPose.y) +
          50 * std::abs(diffPose.theta);
      pose = pose * diffPose;
      lastArduinoPose = *arduinoPose;
    }

    bool capturedFrame = false;
    if (waypoints.front().reached(pose)) {
      if (waypoints.front().visualPosition) {
        capturedFrame = true;
        positionVisual();
      }
      waypoints.pop();
    }

    if (waypoints.empty()) {
      if (!capturedFrame)
        captureFrameBlocking();
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

  sendCommands({84, 0});

  return 0;
}
