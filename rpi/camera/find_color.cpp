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

struct Segment {
  Point start;
  Point end;
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

/// Gradient function. Takes a frame and a point and returns the direction of
/// the gradient at that point. Using the Sobel Algorithm.
double directionOfGradientAtPoint(Point x0, const Frame &frame) {
  int x = x0.x;
  int y = x0.y;

  assert(x >= 1 && x < WIDTH - 1);
  assert(y >= 1 && y < HEIGHT - 1);

  // Sobel Kernels
  int Gx[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};

  int Gy[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};

  double gx = 0, gy = 0;

  // Apply Sobel filter
  for (int i = -1; i <= 1; ++i) {
    for (int j = -1; j <= 1; ++j) {
      HSVPixel pixel = frame.HSV[(y + i) * WIDTH + HEIGHT];

      // This is only an approximation but it should be fine
      double greyscale = (double)pixel.v / 255.;

      // Convolve with Sobel kernels
      gx += greyscale * Gx[i + 1][j + 1];
      gy += greyscale * Gy[i + 1][j + 1];
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
  return pixel.s > int(0.55 * 255) && pixel.v > int(0.35 * 255);
}

bool isBlue(const HSVPixel pixel) {
  return int(210 / 360. * 255) <= pixel.h && pixel.h <= int(260 / 360. * 255) &&
         isColor(pixel);
}

bool isOrange(const HSVPixel pixel) {
  return int(18 / 360. * 255) <= pixel.h && pixel.h <= int(35 / 360. * 255) &&
         isColor(pixel);
}

bool isBlack(const HSVPixel pixel) { return pixel.v < int(0.2 * 255); }

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
  for (int s = -1; s <= 1; s++) {
    for (int i = -1; i <= 1; i++) {
      HSVPixel &pixel = frame.HSV[(x + s) + (y + i) * WIDTH];
      if (isBlack(pixel)) {
        blackPointCounter += 1;
      }
      if (isWhite(pixel)) {
        whitePointCounter += 1;
      }
    }
  }

  return (whitePointCounter >= 3) && (blackPointCounter >= 3);
}
