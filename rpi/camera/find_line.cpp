#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <vector>

#include "camera.h"
#include "find_color.cpp"

struct ScreenLine {
  double angle;
  double distancetoorigin;
};

/// This function takes a vector of points which were detected to be of a
/// specific color. This function draws the best line through this points.
/// Assumptions: There is only one line.
/// Assumptions: The points outside the field are ment to be already filtered
/// out.
ScreenLine findLine(std::vector<Point> points) {
  if (points.size() < 10) {
    std::cerr << "Unfortunately there were to few points" << std::endl;
    assert(false);
  }

  int distanceThreshold = 10;

  std::vector<ScreenLine> lines;

  for (int i = 0; i < int(points.size() / 2); ++i) {
    int index1 = rand() % int(points.size());
    int index2 = rand() % int(points.size());

    int Δx = points[index1].x - points[index2].x;
    int Δy = points[index1].y - points[index2].y;
    int distance_sq = Δx * Δx + Δy * Δy;

    if (distance_sq <= distanceThreshold)
      continue; // Skip if points are too close

    double angle = std::atan2(Δy, Δx); // Calculate angle of line
    if (angle < 0)
      angle += M_PI;
    assert(angle >= 0 && angle <= M_PI); // Ensure angle is in range [0, π]
    double distanceToOrigin =
        std::abs(-points[index1].x * Δy + points[index1].y * Δx) /
        std::sqrt(distance_sq); // Calculate distance to origin

    lines.push_back(
        ScreenLine{angle, distanceToOrigin}); // Store line parameters
  }

  std::sort(lines.begin(), lines.end(),
            [](const ScreenLine &a, const ScreenLine &b) {
              return a.angle < b.angle; // Sort lines by angle
            });

  int countMax = -1;
  int indexMax = -1;

  for (int i = 0; i != int(lines.size()); ++i) {
    // double ref = lines[i].angle;
    int count = 1;
    while (count < int(lines.size()) &&
           lines[(i + count) % lines.size()].angle +
                   (i + count >= int(lines.size()) ? M_PI : 0) -
                   lines[i].angle <
               0.1)
      ++count;
    if (count > countMax) {
      countMax = count;
      indexMax = i;
    }
  }

  double sum = 0;

  for (auto seg : lines) {
    sum += seg.distancetoorigin;
  }
  sum /= lines.size();

  std::cerr << "we made it through" << std::endl;

  return ScreenLine{lines[indexMax].angle, sum};
}

/// This function takes a frame and a line and draws the line in the frame.
/// It colors the line in green.
/// WARNING: This function modifies the frame that is passed.
void colorLineInFrame(Frame &frame, const ScreenLine &Line) {
  std::cerr << "AAAsupi" << std::endl;
  int x1 = sin(Line.angle) * Line.distancetoorigin;
  int y1 = cos(Line.angle) * Line.distancetoorigin;
  std::cerr << "BBBsupi" << std::endl;

  double m = tan(Line.angle);

  for (int x = 0; x < WIDTH; ++x) {
    int y = m * (x - x1) + y1;
    if (y >= 0 && y < HEIGHT) {
      std::cerr << "supi" << std::endl;
      HSVPixel &pixel = frame.HSV[y * WIDTH + x];
      pixel.h = 200;
      pixel.s = 255;
      pixel.v = 255;
    } else
      std::cerr << "rip" << std::endl;
  }

  saveFrame(frame);
}
