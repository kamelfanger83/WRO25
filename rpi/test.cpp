#include <chrono>
#include <iostream>
#include <thread>

#include "camera/camera.h"
#include "camera/find_color.cpp"
#include "camera/find_line.cpp"
#include "geo/camera.cpp"
#include "utils.cpp"
#include "camera/obstacles.cpp"
#include "camera/draw_line.cpp"
#include "geo/coordinatesTrafficlights.cpp"

int main() {
  initializeCamera();

  queueCapture();

  while (lastFrame.timestamp == -1) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  std::cerr << "We captured an image at: " << lastFrame.timestamp << std::endl;

  saveFrame(lastFrame);

  lastFrame.timestamp += 1;

  /* auto gradFrame = cloneFrame(lastFrame);

  for (int x = 1; x < WIDTH - 1; ++x) {
    for (int y = 1; y < HEIGHT - 1; ++y) {
      double angle = directionOfGradientAtPoint({x, y}, lastFrame);
      gradFrame.HSV[y * WIDTH + x].h = (angle / 2 / M_PI + 0.5) * 255;
    }
  }

  saveFrame(gradFrame);

  lastFrame.timestamp += 1; */

  Pose pose = {110, 50, 2.8};

  auto screenLines = findLines(lastFrame, pose);

  saveFrame(lastFrame);
  lastFrame.timestamp += 1;

  drawScreenLineSet(lastFrame, screenLines);

  saveFrame(lastFrame);
  lastFrame.timestamp += 1;

  // Pose pose = {51.63059936, 150.90123742, M_PI_2};

  pose = unwrap(optimizePose(screenLines, pose, pose)).first;

  printPose(pose);

  drawProjectedLines(lastFrame, pose);

  checkTrafficLight(lastFrame, getCameraSystem(pose) , getTrafficLightCoordinates(TrafficLight::TRAFFICLIGHT_1));


  saveFrame(lastFrame);
  lastFrame.timestamp += 1;

  cleanCamera();
}
