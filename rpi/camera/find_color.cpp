#include "camera.h"
#include <cassert>
#include <cmath>
#include <cstdint>
#include <vector>

#include "save_image.cpp"

// The main justification for this struct is that otherwise there is no nice
// option to return RGB values from a function
struct Pixel {
  uint8_t &r, &g, &b;
};

struct Point {
  int x;
  int y;
};

struct Segment {
  Point start;
  Point end;
};

/// Gets a mutable reference to the pixel at (x, y).
Pixel getPixel(Frame &frame, int x, int y) {
  int i = (y * WIDTH + x) * 4;
  return Pixel{frame.XRGB[i + 2], frame.XRGB[i + 1], frame.XRGB[i]};
}

/// Gets a const reference to the pixel at (x, y).
const Pixel getPixel(const Frame &frame, int x, int y) {
  int i = (y * WIDTH + x) * 4;
  return Pixel{const_cast<uint8_t &>(frame.XRGB[i + 2]),
               const_cast<uint8_t &>(frame.XRGB[i + 1]),
               const_cast<uint8_t &>(frame.XRGB[i])};
}

/// Goes over a frame and returns a vector containing all the points in the
/// Frame of a certain color.
std::vector<Point> mask(const Frame &frame, bool (*is_color)(Pixel)) {
  std::vector<Point> points;
  for (int x = 0; x < WIDTH; ++x) {
    for (int y = 0; y < HEIGHT; ++y) {
      if (is_color(getPixel(frame, x, y))) {
        points.push_back({x, y});
      }
    }
  }
  return points;
}

/// Test function for color recognition, colors in the Pixels that are
/// recognized to be of a certain color in neon green and saves the frame as a
/// png.
/// CAUTION: This function modifies the frame that is passed.
void colorColor(Frame &frame, std::vector<Point> points) {
  for (Point p : points) {
    Pixel pixel = getPixel(frame, p.x, p.y);
    pixel.r = 0;
    pixel.g = 255;
    pixel.b = 0;
  }
  saveFrame(frame);
}

/// Converts RGB to greyscale value
double convertRGB(uint8_t R, uint8_t G, uint8_t B) {
  int grayscaleValue = 0.3 * R + 0.59 * G + 0.11 * B;
  return grayscaleValue;
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
      Pixel pixel = getPixel(frame, x + j, y + i);

      double greyscale = convertRGB(pixel.r, pixel.g, pixel.b);

      // Convolve with Sobel kernels
      gx += greyscale * Gx[i + 1][j + 1];
      gy += greyscale * Gy[i + 1][j + 1];
    }
  }

  return std::atan2(gy, gx);
}

/// Whole block of color recognition functions.
bool isBlue(const Pixel pixel) {
  if (pixel.b > 150 && pixel.b / 2 >= pixel.r && pixel.b / 2 >= pixel.g) {
    return true;
  }
  return false;
}

bool isOrange(const Pixel pixel) {

  if (pixel.g / 2 >= pixel.b && pixel.r >= pixel.g + 20 && pixel.r >= 200) {
    return true;
  }
  return false;
}

bool isBlack(const Pixel pixel) {

  if (pixel.r + pixel.g + pixel.b <= 150) {
    return true;
  }
  return false;
}
bool isWhite(const Pixel pixel) {

  if (pixel.r + pixel.g + pixel.b >= 600) {
    return true;
  }
  return false;
}

bool isGreen(const Pixel pixel) {

  if (pixel.g > 150 && pixel.g / 2 >= pixel.r && pixel.g / 2 >= pixel.b) {
    return true;
  }
  return false;
}

bool isRed(const Pixel pixel) {

  if (pixel.r > 150 && pixel.r / 2 >= pixel.g && pixel.r / 2 >= pixel.b) {
    return true;
  }
  return false;
}
