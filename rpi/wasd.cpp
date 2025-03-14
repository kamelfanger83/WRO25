#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

#include "serial.cpp"

int main() {
    if (initializeSerial()) return 1;

    readArduinoResponse();

    // Configure terminal for reading keys
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    std::cout << "Control with WASD and E for going straight. Press 'q' to quit.\n";

    int ang = 90;
    // between 0 and 240
    int speed = 0;

    bool run = true;
    while (run) {
        readArduinoResponse();

        int key = getchar();
        switch (key) {
            case 'q': run = false; break;
            case 'w': speed = std::min(255, speed + 40); break;
            case 's': speed = std::max(-255, speed - 40); break;
            case 'a': ang = std::min(170, ang + 15); break;
            case 'd': ang = std::max(0, ang - 15); break;
            case 'e': ang = 90; break;
        }

        sendCommands(ang, speed);

        readArduinoResponse();
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return 0;
}
