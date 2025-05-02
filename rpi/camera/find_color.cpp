#pragma once

#include "camera.h"
#include <cassert>
#include <cmath>
#include <vector>

#include "save_image.cpp"

struct Point {
  int x;
  int y;
};

/// Goes over a frame and returns a vector containing all the points in the
/// Frame of a certain color.
std::vector<Point> mask(const Frame &frame, bool (*is_color)(const HSVPixel)) {
  std::vector<Point> points;
  for (int x = 0; x < WIDTH; ++x) {
    for (int y = 0; y < HEIGHT; ++y) {
      if (is_color(frame.HSV[y * WIDTH + x])) {
        points.push_back({x, y});
      }
    }
  }
  return points;
}

/// Test function for color recognition, colors in the Pixels that are
/// passed in the given color.
void colorColor(Frame &frame, std::vector<Point> points,
                HSVPixel color = {100, 255, 255}) {
  for (Point p : points) {
    frame.HSV[p.y * WIDTH + p.x] = color;
  }
}

/// Gradient function using 5x5 Sobel kernel. Takes a frame and a point and
/// returns the direction of the gradient at that point.
double directionOfGradientAtPoint(Point x0, const Frame &frame) {
  int x = x0.x;
  int y = x0.y;

  assert(x >= 2 && x < WIDTH - 2);
  assert(y >= 2 && y < HEIGHT - 2);

  // 5x5 Sobel Kernels
  int Gx[5][5] = {{-1, -2, 0, 2, 1},
                  {-4, -8, 0, 8, 4},
                  {-6, -12, 0, 12, 6},
                  {-4, -8, 0, 8, 4},
                  {-1, -2, 0, 2, 1}};

  int Gy[5][5] = {{-1, -4, -6, -4, -1},
                  {-2, -8, -12, -8, -2},
                  {0, 0, 0, 0, 0},
                  {2, 8, 12, 8, 2},
                  {1, 4, 6, 4, 1}};

  double gx = 0, gy = 0;

  // Apply 5x5 Sobel filter
  for (int i = -2; i <= 2; ++i) {
    for (int j = -2; j <= 2; ++j) {
      HSVPixel pixel = frame.HSV[(y + i) * WIDTH + (x + j)];

      double greyscale = double(pixel.v) / 255.;

      gx += greyscale * Gx[i + 2][j + 2];
      gy += greyscale * Gy[i + 2][j + 2];
    }
  }

  return std::atan2(gy, gx);
}

/// Test function for gradient. Returns a vector of points
/// whose gradient is in the range of a choosen lower and upper bound
std::vector<Point> gradientPoints(const Frame &frame, double lowerBound,
                                  double upperBound) {
  std::vector<Point> points;
  for (int x = 0; x < WIDTH; ++x) {
    for (int y = 0; y < HEIGHT; ++y) {
      double angle = directionOfGradientAtPoint({x, y}, frame);
      if (lowerBound <= angle && angle <= upperBound) {
        points.push_back({x, y});
      }
    }
  }
  return points;
}

// Whole block of color recognition functions.

/// Returns whether a pixel is considered to have a specific color rather than
/// some mix or some shade of grey.
bool isColor(const HSVPixel pixel) {
  return pixel.s > int(0.40 * 255) && pixel.v > int(0.35 * 255);
}

bool isBlue(const HSVPixel pixel) {
  return int(215 / 360. * 255) <= pixel.h && pixel.h <= int(260 / 360. * 255) &&
         isColor(pixel);
}

bool isOrange(const HSVPixel pixel) {
  return int(10 / 360. * 255) <= pixel.h && pixel.h <= int(35 / 360. * 255) &&
         isColor(pixel);
}

bool isBlack(const HSVPixel pixel) {
  // return pixel.v < int(0.2 * 255);
  return int(190 / 360. * 255) <= pixel.h && pixel.h <= int(215 / 360. * 255) &&
         isColor(pixel);
}

bool isWhite(const HSVPixel pixel) {
  return pixel.s < int(0.15 * 255) && pixel.v > int(0.75 * 255);
}

bool isGreen(const HSVPixel pixel) {
  return int(95 / 360. * 255) <= pixel.h && pixel.h <= int(130 / 360. * 255) &&
         isColor(pixel);
}

bool isRed(const HSVPixel pixel) {
  return (int(350 / 360. * 255) <= pixel.h ||
          pixel.h <= int(10 / 360. * 255)) &&
         isColor(pixel);
}

bool inBounds(const Frame &frame, const Point point) {
  return point.x >= 0 && point.x < WIDTH && point.y >= 0 && point.y < HEIGHT;
}

/// returns wether or not a pixel is a border point
bool isBorderPoint(const Frame &frame, const Point point) {
  int x = point.x;
  int y = point.y;

  int blackPointCounter = 0;
  int whitePointCounter = 0;
  for (int s = -2; s <= 2; s++) {
    for (int i = -2; i <= 2; i++) {
      Point potential;
      potential.x = x + s;
      potential.y = y + i;

      if (!inBounds(frame, potential)) {
        return false;
      }

      HSVPixel &pixel = frame.HSV[potential.y * WIDTH + potential.x];
      if (isBlack(pixel)) {
        blackPointCounter += 1;
      }
      if (isWhite(pixel)) {
        whitePointCounter += 1;
      }
    }
  }

  return (whitePointCounter >= 5) && (blackPointCounter >= 5);
}
