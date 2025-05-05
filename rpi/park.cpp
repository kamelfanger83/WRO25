#pragma once

#include <iostream>
#include <chrono>
#include <thread>

#include "structs.h"
#include "serial.cpp"


// direction 1 --> forward
// direction 2 --> backward
void moveX(Commands& commands, double threshold, bool dir){

    if (dir) {
        commands.speed = 50;
    } else {
        commands.speed = -30;
    }
    sendCommands(commands);

    while (true){
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        auto pose = processArduinoResponse();
        if (dir){
            if (pose.has_value() && pose->x > threshold){
                commands.speed = 0;
                sendCommands(commands);
                std::cout << "moved to x = " << pose->x << std::endl;
                break;
            }
        } else {
            if (pose.has_value() && pose->x < threshold){
                commands.speed = 0;
                sendCommands(commands);
                std::cout << "moved to x = " << pose->x << std::endl;
                break;
            } 
        }
    }
}

void moveY(Commands& commands, double threshold, bool dir){

    if (dir) {
        commands.speed = 50;
    } else {
        commands.speed = -30;
    }
    sendCommands(commands);

    while (true){
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        auto pose = processArduinoResponse();
        if (dir){
            if (pose.has_value() && pose->y < threshold){
                commands.speed = 0;
                sendCommands(commands);
                std::cout << "moved to y = " << pose->y << std::endl;
                break;
            }
        } else {
            if (pose.has_value() && pose->y > threshold){
                commands.speed = 0;
                sendCommands(commands);
                std::cout << "moved to y = " << pose->y << std::endl;
                break;
            } 
        }
    }
}

void setAngle(Commands& commands, int angle){
    commands.angle = angle;
    sendCommands(commands);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << "set angle to " << angle << std::endl;
}


void unpark(){

    const double base_angle = 84;

    resetBlind();
    std::cout << "reset blind" << std::endl;

    //Pose pose = {0, 0, 0};
    Commands commands {84, 0};

    setAngle(commands, 0);

    moveX(commands, 9, 1);

    setAngle(commands, 140);

    moveY(commands, -6.5, 0);

    setAngle(commands, 0);

    moveY(commands, -15, 1);


    setAngle(commands, base_angle);

    resetBlind();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << "reached end" << std::endl;
}

void park(){
    resetBlind();
    resetBlind();
}