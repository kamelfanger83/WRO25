#include <chrono>
#include <iostream>
#include <thread>

#include "camera.h"
#include "find_color.cpp"
#include "find_line.cpp"

int main() {
  initializeCamera();

  while (lastFrame.timestamp == -1) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  std::cerr << "We captured an image at: " << lastFrame.timestamp << std::endl;

  auto amask = mask(lastFrame, isOrange);
  colorColor(lastFrame, amask);
  drawLineInFrame(lastFrame, *findLine(amask));

  saveFrame(lastFrame);

  cleanCamera();
}
