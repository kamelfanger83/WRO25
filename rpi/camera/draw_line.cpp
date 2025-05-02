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
