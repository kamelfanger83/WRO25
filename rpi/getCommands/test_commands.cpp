#pragma once

#include <iostream>
#include "../structs.h"
#include "getCommands.cpp"
int main() {
    ControllerState state;
    Waypoint target;

    std::cout << "Ziel-Waypoint eingeben (x y): ";
    std::cin >> target.x >> target.y;

    while (true) {
        Pose current_pose;
        std::cout << "\nAktuelle Pose eingeben (x y theta_in_deg): ";
        double theta_deg;
        std::cin >> current_pose.x >> current_pose.y >> theta_deg;
        current_pose.theta = theta_deg * M_PI / 180.0;

        Commands cmd = getCommands(current_pose, target, state);

        std::cout << "→ Lenkwinkel (angle): " << cmd.angle
                  << " | Geschwindigkeit (speed): " << cmd.speed << "\n";
    }
}
