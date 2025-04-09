#pragma once

#include "../structs.h"
#include <math.h>
#include <iostream>

Commands getCommands(const Pose& pose, const Waypoint& waypoint, ControllerState& state){
    const double Kp = 40; //proportional gain, has to be tested on vehicle to be calibrated
    const double Kd = 20; //differential gain, same as above
    const double base_speed = 200;
    const double base_angle = 90;
    
    double dx = waypoint.x - pose.x;
    double dy = waypoint.y - pose.y;
    double target_angle = atan2(dy, dx);

    double angle_error = target_angle - pose.theta;
    //make sure angle_error is within +- pi
    while (angle_error > M_PI) angle_error -= 2 * M_PI;
    while (angle_error < -M_PI) angle_error += 2 * M_PI;

    double derivative = angle_error - state.prev_error;
    state.prev_error = angle_error;

    double steering = Kp * angle_error + Kd * derivative;
    int angle_command = static_cast<int>(base_angle - steering); 
    //Our handles angles counterintuitively. Maybe change this in
    //serial.cpp such that everything above 90 is a left turn and everything below is a right turn?
    angle_command = std::max(0, std::min(170, angle_command));

    int speed_command = base_speed - std::abs(steering) * 2; //Has to be calibrated with the vehicle
    speed_command = std::max(-255, std::min(255, speed_command));

    return {angle_command, speed_command};
}
