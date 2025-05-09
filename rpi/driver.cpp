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

int run(const Mode &startMode, const Pose &startPose, bool openRound) {
  std::queue<Waypoint> waypoints;
  Mode nextMode = startMode;
  Pose pose = startPose;
  Pose lastArduinoPose = {0, 0, 0};
  ControllerState controllerState{0};

  bool first = true;
  Commands commands{84, 0};

  const int blurSleep = 500;

  auto positionVisual = [&]() -> void {
    first = false;
    commands.speed = 0;
    sendCommands(commands);

    std::this_thread::sleep_for(std::chrono::milliseconds(blurSleep));

    captureFrameBlocking();
    std::cout << "Captured a frame at: " << lastFrame.timestamp << std::endl;

    saveFrame(lastFrame);
    lastFrame.timestamp += 1;

    std::cout << "Pre visual pose:\n";
    printPose(pose);
    auto screenLines = findLines(lastFrame, pose);
    drawScreenLineSet(lastFrame, screenLines);
    if (openRound)
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
      if (flipped) {
        diffPose.theta *= -1;
        diffPose.y *= -1;
      }
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
      if (!capturedFrame && !openRound) {
        commands.speed = 0;
        sendCommands(commands);
        std::this_thread::sleep_for(std::chrono::milliseconds(blurSleep));
        captureFrameBlocking();
      }
      if (openRound && first) {
        positionVisual();
      }
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
