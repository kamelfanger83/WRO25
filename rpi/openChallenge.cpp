#include "driver.cpp"

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

  waypoints.push({50, 228});
  waypoints.push({228, 250});
  waypoints.push({250, 72});
  waypoints.push({78, 50});
  waypoints.push(startPoint);

  return {std::make_pair(waypoints, Mode{endMode})};
}

int main() { return run({startMode}); }
