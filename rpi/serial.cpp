#pragma once

#include <cassert>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <optional>
#include <regex>
#include <termios.h>
#include <unistd.h>

#include "structs.h"

const std::string port = "/dev/ttyUSB0";
const int baud = B115200;

// its a bit cringe to have globals in cpp file which is meant to be included
// but whatever
int serial_port;
bool initialized = false;

/// Returns 0 if successful, returns nonzero number otherwise.
int initializeSerial() {
  if (initialized)
    return 0;

  // Open the serial port (no O_NONBLOCK now)
  serial_port = open(port.c_str(), O_RDWR);
  if (serial_port < 0) {
    std::cerr << "Failed to open port! Error: " << strerror(errno) << std::endl;
    return 1;
  }

  // Configure the serial connection
  struct termios tty;
  if (tcgetattr(serial_port, &tty) != 0) {
    std::cerr << "Error getting terminal attributes" << std::endl;
    return 2;
  }

  // Set baud rate
  cfsetispeed(&tty, baud);
  cfsetospeed(&tty, baud);

  // It is not clear which of these configurations are strictly necessary.
  // However with the ones here it works so we can leave it as is.
  tty.c_cflag &= ~PARENB; // No parity
  tty.c_cflag &= ~CSTOPB; // One stop bit
  tty.c_cflag &= ~CSIZE;
  tty.c_cflag |= CS8;            // 8 bits per byte
  tty.c_cflag |= CREAD | CLOCAL; // Enable read, disable ctrl lines
  tty.c_lflag &=
      ~(ICANON | ECHO | ECHOE | ISIG);    // Set non-canonical mode (raw mode)
  tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Disable software flow control

  // This one is for sure necessary.
  // Set non-blocking (read returns if no data can be read atm)
  tty.c_cc[VMIN] = 0;  // Return immediately if no data
  tty.c_cc[VTIME] = 0; // No timeout

  // Apply the new settings
  if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
    std::cerr << "Error setting terminal attributes" << std::endl;
    return 3;
  }

  // Flush any data already received but not read
  if (tcflush(serial_port, TCIFLUSH) != 0) {
    std::cerr << "Error flushing serial port input buffer" << std::endl;
    return 4;
  }

  std::string message = "6969\n";
  write(serial_port, message.c_str(), message.length());

  initialized = true;
  return 0;
}

/// Reads data sent from Arduino and prints it to stdout. Additionally looks for
/// poses reported by the Arduino and updates arduinoPose.
std::optional<Pose> processArduinoResponse() {
  char buf[2048];
  int n = read(serial_port, buf, sizeof(buf) - 1);

  if (n < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      // No data available right now; return without printing.
      return {};
    }
    std::cerr << "Error reading from the serial port." << std::endl;
    return {};
  } else if (n > 0) {

    buf[n] = '\0'; // Null-terminate the received data
    std::string buffer = std::string(buf);

    std::cerr << "[ARDUINO]: " << buffer;

    std::string latestPacket;

    // Regex to find all matching [x=...,y=...,t=...] patterns
    std::regex packetRegex(
        R"(\[x=([-+]?[0-9]*\.?[0-9]+),y=([-+]?[0-9]*\.?[0-9]+),t=([-+]?[0-9]*\.?[0-9]+)\])");
    std::smatch match;
    auto searchStart = buffer.cbegin();
    while (std::regex_search(searchStart, buffer.cend(), match, packetRegex)) {
      latestPacket = match.str(); // Keep updating with the latest match
      searchStart = match.suffix().first;
    }

    if (!latestPacket.empty()) {
      std::smatch values;
      if (std::regex_match(latestPacket, values, packetRegex)) {
        float x = std::stof(values[1]);
        float y = std::stof(values[2]);
        float theta = std::stof(values[3]);

        return {{x, y, theta}};
      }
    } else {
      return {};
    }
  }
  return {};
}

/// Sends updates to servo angle and motor speed to the Arduino.
/// Acceptable ranges:
///     - 0 <= angle <= 170
///     - -255 <= speed <= 255
/// Returns 0 if successful, returns nonzero number otherwise.
int sendCommands(Commands commands) {
  if (commands.angle < 0 || commands.angle > 170) {
    std::cerr << "Invalid angle" << std::endl;
    return 1;
  }
  if (commands.speed < -255 || commands.speed > 255) {
    std::cerr << "Invalid speed" << std::endl;
    return 2;
  }
  // Prepare message: odd numbers for servo commands, even for motor commands
  std::string message = std::to_string(commands.angle * 2 + 1) + "\n" +
                        std::to_string(commands.speed * 2) + "\n";
  write(serial_port, message.c_str(), message.length());
  return 0;
}
