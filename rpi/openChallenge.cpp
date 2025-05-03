#include "camera/camera.h"
#include "driver.cpp"
#include <cmath>

std::optional<std::pair<std::queue<Waypoint>, Mode>>
endMode(const Frame &frame, const Pose &position) {
  return {};
}

Pose startPose = {50, 150, M_PI_2};
int roundsCompleted = -1;

std::optional<std::pair<std::queue<Waypoint>, Mode>>
startMode(const Frame &frame, const Pose &position) {
  std::queue<Waypoint> waypoints;

  ++roundsCompleted;

  waypoints.push({25, 250});
  waypoints.push({250, 275});
  waypoints.push({275, 50});
  waypoints.push({50, 25});

  if (roundsCompleted == 2) {
    waypoints.push({startPose.x, startPose.y});
    return {{waypoints, Mode{endMode}}};
  } else {

    return {{waypoints, Mode{startMode}}};
  }
}

int main() {
  initializeCamera();

  long long lastTimeStamp = lastFrame.timestamp;
  queueCapture();
  while (lastTimeStamp == lastFrame.timestamp) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  std::cout << "Captured first frame at: " << lastFrame.timestamp << std::endl;

  std::array<Pose, 6> startCandidates = {
      Pose{35, 125, M_PI_2}, {50, 125, M_PI_2}, {65, 125, M_PI_2},
      {35, 175, M_PI_2},     {50, 175, M_PI_2}, {65, 175, M_PI_2}};

  double minLoss = 1e9;
  Pose bestPose = {50, 150, M_PI_2};

  for (auto startCandidate : startCandidates) {
    auto screenLines = findLines(lastFrame, startCandidate);

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

  run({startMode}, bestPose, true);

  cleanCamera();

  return 0;
}
