#include <chrono>
#include <iostream>
#include <thread>

#include "camera.h"
#include "show_image.cpp"

int main() {
  initializeCamera();

  while (lastFrame.timestamp == -1) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  std::cerr << "We captured an image at: " << lastFrame.timestamp << std::endl;

  saveFrame(lastFrame);

  cleanCamera();
}
