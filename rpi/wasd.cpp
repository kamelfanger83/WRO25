#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

#include "serial.cpp"
#include "structs.h"

int main() {
    if (initializeSerial()) return 1;

    processArduinoResponse();

    // Configure terminal for reading keys
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    std::cout << "Control with WASD and E for going straight. Press 'q' to quit.\n";

    Commands commands{90, 0};


    bool run = true;
    while (run) {
        processArduinoResponse();

        int key = getchar();
        switch (key) {
            case 'q': run = false; break;
            case 'w': commands.speed = std::min(255, commands.speed + 40); break;
            case 's': commands.speed = std::max(-255, commands.speed - 40); break;
            case 'a': commands.angle = std::min(170, commands.angle + 15); break;
            case 'd': commands.angle = std::max(0, commands.angle - 15); break;
            case 'e': commands.angle = 90; break;
        }

        sendCommands(commands);

        processArduinoResponse();
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return 0;
}
