#pragma once
#include <cmath>

#include "../structs.h"

/// This function takes a frame and a line and draws the line in the frame in
/// the given color.
void drawLineInFrame(Frame &frame, const ScreenLine &Line,
                     HSVPixel color = {200, 255, 255}) {
  double x1 = -sin(Line.angle) * Line.distanceToOrigin;
  double y1 = cos(Line.angle) * Line.distanceToOrigin;

  double xm = tan(Line.angle);
  for (int x = 0; x < WIDTH; ++x) {
    int y = std::round(xm * (x - x1) + y1);
    if (y >= 0 && y < HEIGHT) {
      frame.HSV[y * WIDTH + x] = color;
    }
  }

  double ym = tan(M_PI_2 - Line.angle);
  for (int y = 0; y < HEIGHT; ++y) {
    int x = std::round(ym * (y - y1) + x1);
    if (x >= 0 && x < WIDTH) {
      frame.HSV[y * WIDTH + x] = color;
    }
  }
}


// PRE: frame, points (= 4 corenterbox on the screen)
// POST: Draws box (= square) around the trafficlights.

void drawTrafficLights(Frame &frame, int minx, int maxx, int miny,
                       int maxy, HSVPixel color = {0, 255, 255}) {

  // check if the points are in the frame and not only on the screen

  //upper, lower
  for (int x = minx; x < maxx; ++x) {
    frame.HSV[miny * WIDTH + x] = color;
    frame.HSV[maxy * WIDTH + x] = color;
  }
  //right, left
  for (int y = miny; y < maxy; ++y) {
    frame.HSV[y * WIDTH + minx] = color;
    frame.HSV[y * WIDTH + maxx] = color;
  }

}
