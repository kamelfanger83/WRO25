#include <chrono>
#include <iostream>
#include <thread>

#include "camera.h"

int main() {
  initializeCamera();

  while (lastFrame.timestamp == -1) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  std::cerr << "We captured an image at: " << lastFrame.timestamp << std::endl;

  cleanCamera();
}
