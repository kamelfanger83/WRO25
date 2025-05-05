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
#include "camera/obstacles.cpp"
#include "geo/coordinatesTrafficlights.cpp"

///have traffic light check function in obstacles.cpp signature: 
/*std::optional<char>
checkTrafficLight(Frame &frame, const CoordinateSystem &cameraSystem,
  const std::vector<Vector> &unprojectedPoints)*/ 


bool flipped = false;

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
  //function to leave the parking spot. 

  std::queue<Waypoint> waypoints;
  Waypoint startPoint;
  startPoint.x = position.x;
  startPoint.y = position.y;
/// three case, no traffic light in middle, red light, green light;


if(!(checkTrafficLight(frame, getCameraSystem(position), getTrafficLightCoordinates(TrafficLight::TRAFFICLIGHT_4)).has_value())){
  waypoints.push({});
  waypoints.push({50, 150});
  return {{waypoints, Mode{modeFromMiddle}}};
}else if((checkTrafficLight(frame, getCameraSystem(position), getTrafficLightCoordinates(TrafficLight::TRAFFICLIGHT_4)).has_value())=='r'){
  waypoints.push({});
  waypoints.push({});
  waypoints.push({});
  waypoints.push({80, 200});
  return {{waypoints, Mode{modeFromEndRight}}};
}else{
  waypoints.push({});
  waypoints.push({})
  waypoints.push({20, 200});
  waypoints.push({30, 228});

  return {{waypoints, Mode{modefromEndLeft}}};
}
}
