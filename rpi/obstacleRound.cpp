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
#include "park.cpp"



std::optional<char> checkTrafficLightInSegment(Frame &frame, Segment s, Pose &position, TrafficLight tl){
  auto coords = getTrafficLightCoordinates(tl);
  for(Vector &corner : coords){
    corner = vectorInSegment(s,corner);
  }
  return checkTrafficLight(frame, getCameraSystem(position), coords);

}
///have traffic light check function in obstacles.cpp signature: 
/*std::optional<char>
checkTrafficLight(Frame &frame, const CoordinateSystem &cameraSystem,
  const std::vector<Vector> &unprojectedPoints)*/ 


bool flipped = false;

struct Mode {
  std::function<std::optional<std::pair<std::queue<Waypoint>, Mode>>(
      Frame &, const Pose &)>
      plan;
};

std::optional<std::pair<std::queue<Waypoint>, Mode>>
endMode(Frame &frame, const Pose &position) {
  return {};
}

std::optional<std::pair<std::queue<Waypoint>, Mode>>
startMode(Frame &frame, const Pose &position) { 

  std::queue<Waypoint> waypoints;

/// three case, no traffic light in middle, red light, green light;
//function to leave the parking spot. 
unpark();

if(!(checkTrafficLight(frame, getCameraSystem(position), getTrafficLightCoordinates(TrafficLight::TRAFFICLIGHT_4)).has_value())){
  waypoints.push({});
  waypoints.push({50, 150});
  return {{waypoints, Mode{modeFromMiddle}}};
}else if((checkTrafficLight(frame, getCameraSystem(position),  getTrafficLightCoordinates(TrafficLight::TRAFFICLIGHT_4)))=='r'){
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
  return {{waypoints, Mode{modeFromEndLeft}}};
}
}


/// mode from the middle, 
std::optional<std::pair<std::queue<Waypoint>, Mode>> 
modeFromMiddle(Frame &frame, const Pose &position){

  /// cases: on position 5 or on position 6. left or right.
 Segment curr = inSegment(position);
 std::queue<Waypoint> waypoints;

 if(!(checkTrafficLightInSegment(frame, curr, position, TrafficLight::TRAFFICLIGHT_5).has_value() &&
    !(checkTrafficLightInSegment(frame, curr, position, TrafficLight::TRAFFICLIGHT_6)).has_value()){
  waypoints.push(wayPointInSegment(curr, {20, 200}));
  waypoints.push(wayPointInSegment(curr, {30, 228}))
}

}

