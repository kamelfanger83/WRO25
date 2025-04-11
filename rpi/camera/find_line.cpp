#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <optional>
#include <vector>

#include "../structs.h"
#include "camera.h"
#include "find_color.cpp"

/// This function takes a vector of points which were detected to be of a
/// specific color. This function draws the best line through this points.
/// Assumptions: There is only one line.
/// Assumptions: The points outside the field are ment to be already filtered
/// out.
std::optional<ScreenLine> findLine(std::vector<Point> points) {
  if (points.size() < 10) {
    std::cerr << "Insufficient points to find a line." << std::endl;
    return {};
  }

  struct ProbedLine {
    Point *p1, *p2;
    double angle;
  };

  std::vector<ProbedLine> lines;

  const int distanceThreshold = 100;
  const int targetLineCount = 2000;

  for (int tries = 0; lines.size() < targetLineCount && tries < 10000;
       ++tries) {
    Point &p1 = points[rand() % int(points.size())];
    Point &p2 = points[rand() % int(points.size())];

    int Δx = p1.x - p2.x;
    int Δy = p1.y - p2.y;
    int distance_sq = Δx * Δx + Δy * Δy;

    if (distance_sq <= distanceThreshold)
      continue; // Skip if points are too close

    double angle = std::atan2(Δy, Δx); // Calculate angle of line
    if (angle < 0)
      angle += M_PI;
    assert(angle >= 0 && angle <= M_PI); // Ensure angle is in range [0, π]

    lines.push_back({&p1, &p2, angle}); // Store line parameters
  }

  if (lines.size() < targetLineCount) {
    std::cerr << "The points were too close together, insufficient lines could "
                 "be formed"
              << std::endl;
    return {};
  }

  std::sort(lines.begin(), lines.end(),
            [](const ProbedLine &a, const ProbedLine &b) {
              return a.angle < b.angle; // Sort lines by angle
            });

  int countMax = -1;
  double maxRangeL, maxRangeR;

  const float MAX_DIF = 0.02;

  for (int i = 0, r = 1; i < int(lines.size()); ++i) {
    if (r == i)
      r = i + 1;
    while ((r + 1) % lines.size() != unsigned(i) &&
           lines[r].angle + (r < i ? M_PI : 0) - lines[i].angle < MAX_DIF) {
      r = (r + 1) % lines.size();
    }
    int count = r - i;
    if (r < i) {
      count = r + lines.size() - i;
    }
    if (count > countMax) {
      countMax = count;
      maxRangeL = lines[i].angle;
      if (r > i)
        maxRangeR = lines[r - 1].angle;
      else if (r == 0)
        maxRangeR = lines[lines.size() - 1].angle;
      else
        maxRangeR = lines[r - 1].angle + M_PI;
    }
  }

  double finalAngle = (maxRangeL + maxRangeR) / 2;

  if (finalAngle >= M_PI)
    finalAngle -= M_PI;

  // Find averge distanceToOrigin of all lines which fall into the angle range.
  double sum = 0, cnt = 0;

  for (auto seg : lines) {
    // Ensure that both cases (normal angle range or angle range which goes
    // above PI) are handled correctly.
    if ((maxRangeL <= seg.angle && seg.angle <= maxRangeR) ||
        (maxRangeL <= seg.angle + M_PI && seg.angle + M_PI <= maxRangeR)) {

      auto getDistanceToOrigin = [finalAngle](Point &p) -> double {
        return -std::sin(finalAngle) * p.x + std::cos(finalAngle) * p.y;
      };
      double distanceToOrigin =
          (getDistanceToOrigin(*seg.p1) + getDistanceToOrigin(*seg.p2)) / 2.;
      sum += distanceToOrigin;
      cnt += 1.;
    }
  }

  if (cnt < 50.) {
    std::cerr << "Too few lines were in the best angle range." << std::endl;
    return {};
  }

  return ScreenLine{finalAngle, sum / cnt};
}

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


/// This function takes a frame, a line and a point and says if the point is below the line.
bool isBelow(Frame &frame, const ScreenLine &Line,
  Point &point) {

  double x1 = -sin(Line.angle) * Line.distanceToOrigin;
  double y1 = cos(Line.angle) * Line.distanceToOrigin;

  double xm = tan(Line.angle);
  int x = point.x;

  int y_line = std::round(xm * (point.x - x1) + y1);

  return (point.y > y_line);
}



/// This function takes a frame, a line and a point and says if the point is right the line.
bool isRight(Frame &frame, const ScreenLine &Line,
  Point &point) {

  double x1 = -sin(Line.angle) * Line.distanceToOrigin;
  double y1 = cos(Line.angle) * Line.distanceToOrigin;

  double ym = tan(M_PI_2 - Line.angle);

  int x_line = std::round(ym * (point.y - y1) + x1);

  return (point.x > x_line);
  }


// WIP
ScreenLineSet analyseFrame(const Frame &frame) {
  ScreenLineSet lines;

  unsigned int threshold =
      250; // number of points needed to say that there is a line to be analysed
  auto bluepoints = mask(frame, isBlue);
  auto orangepoints = mask(frame, isOrange);

  // stores colored line iff it ∃ (= enough points of that color).
  if (bluepoints.size() >= threshold)
    lines.blue = findLine(bluepoints);
  if (orangepoints.size() >= threshold)
    lines.orange = findLine(orangepoints);

  /*
  Idea how to treat gradient:
  make a threshold for the magnitude of the gradient to consider to be a border
  of the field.

  */

  return lines;
}
