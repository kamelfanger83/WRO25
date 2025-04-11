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

  auto screenLines = findLines(lastFrame);

  if (screenLines.left.has_value())
    drawLineInFrame(lastFrame, *screenLines.left, {212, 255, 255});
  else
    std::cerr << "Unfortunately, there was no left line." << std::endl;
  if (screenLines.back.has_value())
    drawLineInFrame(lastFrame, *screenLines.back, {43, 255, 255});
  else
    std::cerr << "Unfortunately, there was no back line." << std::endl;
  if (screenLines.right.has_value())
    drawLineInFrame(lastFrame, *screenLines.right, {85, 255, 255});
  else
    std::cerr << "Unfortunately, there was no right line." << std::endl;
  if (screenLines.orange.has_value())
    drawLineInFrame(lastFrame, *screenLines.orange, {127, 255, 255});
  else
    std::cerr << "Unfortunately, there was no orange line." << std::endl;
  if (screenLines.blue.has_value())
    drawLineInFrame(lastFrame, *screenLines.blue, {0, 255, 255});
  else
    std::cerr << "Unfortunately, there was no blue line." << std::endl;

  saveFrame(lastFrame);

  cleanCamera();
}
