#pragma once

#include "../structs.h"
#include <cmath>
#include <iostream>
#include <math.h>

Commands getCommands(const Pose &pose, const Waypoint &waypoint,
                     ControllerState &state) {
  double Kp = 100;
  const double base_speed = 40;
  const double base_angle = 84;

  double dx = waypoint.x - pose.x;
  double dy = waypoint.y - pose.y;

 /* if (std::sqrt(dx * dx + dy * dy) < 5.)
    Kp = 1.;*/

  double target_angle = atan2(dy, dx);

  double angle_error = target_angle - pose.theta;
  // make sure angle_error is within +- pi
  while (angle_error > M_PI)
    angle_error -= 2 * M_PI;
  while (angle_error < -M_PI)
    angle_error += 2 * M_PI;

  double steering = Kp * angle_error;
  int angle_command = base_angle + steering;
  angle_command = std::max(0, std::min(168, angle_command));

  int speed_command = base_speed;

  return {angle_command, speed_command};
}
