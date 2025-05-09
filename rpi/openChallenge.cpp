#include "camera/camera.h"
#include "camera/find_color.cpp"
#include "camera/find_line.cpp"
#include "driver.cpp"
#include <cmath>

bool flipped = false;

std::optional<std::pair<std::queue<Waypoint>, Mode>>
endMode(const Frame &frame, const Pose &position) {
  return {};
}

Pose startPose = {50, 150, M_PI_2};
int roundsCompleted = -1;

std::optional<std::pair<std::queue<Waypoint>, Mode>>
startMode(const Frame &frame, const Pose &position) {
  std::queue<Waypoint> waypoints;

  std::cout << "Startpose: ";
  printPose(startPose);

  ++roundsCompleted;

  if (roundsCompleted == 0) {
    startPose = position;
    waypoints.push({startPose.x, 240, false});
  } else {
    waypoints.push({30, 240, false});
  }
  waypoints.push({150, 270, true});
  waypoints.push({240, 270, false});
  waypoints.push({270, 150, true});
  waypoints.push({270, 60, false});
  waypoints.push({150, 30, true});
  waypoints.push({60, 30, false});

  if (roundsCompleted == 2) {
    waypoints.push({30, 100, true});
    waypoints.push({startPose.x, startPose.y});
    return {{waypoints, Mode{endMode}}};
  } else {
    waypoints.push({30, 150, true});
    return {{waypoints, Mode{startMode}}};
  }
}

int main() {
  initializeCamera();
  initializeSerial();

  waitForGo();

  captureFrameBlocking();

  flipped = isFlipped(lastFrame);

  captureFrameBlocking();

  std::cout << "Captured first frame at: " << lastFrame.timestamp << std::endl;

  std::array<Pose, 6> startCandidates = {
      Pose{35, 125, M_PI_2}, {50, 125, M_PI_2}, {65, 125, 1.77382},
      {35, 175, M_PI_2},     {50, 175, M_PI_2}, {65, 175, 1.77382}};

  double minLoss = 1e9;
  Pose bestPose = {50, 150, M_PI_2};

  for (auto startCandidate : startCandidates) {
    auto screenLines = findLines(lastFrame, startCandidate);

    drawScreenLineSet(lastFrame, screenLines);

    Pose pose = startCandidate;
    auto poset = optimizePose(screenLines, pose, pose);

    if (poset.has_value()) {
      if (poset->second < minLoss) {
        minLoss = poset->second;
        bestPose = poset->first;
      }
    }
  }

  startPose = bestPose;

  drawProjectedLines(lastFrame, startPose, {43, 255, 255});
  saveFrame(lastFrame);

  run({startMode}, bestPose, true);

  cleanCamera();

  return 0;
}
