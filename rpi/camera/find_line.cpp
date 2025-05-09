#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <optional>
#include <vector>

#include "../geo/camera.cpp"
#include "../geo/coordinates.cpp"
#include "../structs.h"
#include "camera.h"
#include "draw_line.cpp"
#include "find_color.cpp"

/// This function takes a vector of points which were detected to be of a
/// specific color. This function draws the best line through this points.
/// Assumptions: There is only one line.
/// Assumptions: The points outside the field are ment to be already filtered
/// out.
std::optional<ScreenLine> findLine(std::vector<Point> points) {
  if (points.size() < 400) {
    std::cout << "Insufficient points to find a line." << std::endl;
    return {};
  }

  struct ProbedLine {
    Point *p1, *p2;
    double angle;
  };

  std::vector<ProbedLine> lines;

  const int distanceThreshold = 60;
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
    std::cout << "The points were too close together, insufficient lines could "
                 "be formed"
              << std::endl;
    return {};
  }

  std::sort(lines.begin(), lines.end(),
            [](const ProbedLine &a, const ProbedLine &b) {
              return a.angle < b.angle; // Sort lines by angle
            });

  int countMax = -1;
  double maxRangeL = 0., maxRangeR = 0.;

  const float MAX_DIF = 0.04;

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
    std::cout << "Too few lines were in the best angle range." << std::endl;
    return {};
  }

  return ScreenLine{finalAngle, sum / cnt};
}

/// This function takes a frame, a line and a point and says if the point is
/// below the line.
bool isBelow(const ScreenLine &Line, Point &point) {

  double x1 = -sin(Line.angle) * Line.distanceToOrigin;
  double y1 = cos(Line.angle) * Line.distanceToOrigin;

  double xm = tan(Line.angle);

  int y_line = std::round(xm * (point.x - x1) + y1);

  return (point.y > y_line);
}

/// This function takes a frame, a line and a point and says if the point is
/// right of the line.
bool isRight(const ScreenLine &Line, const Point &point) {

  double x1 = -sin(Line.angle) * Line.distanceToOrigin;
  double y1 = cos(Line.angle) * Line.distanceToOrigin;

  double ym = tan(M_PI_2 - Line.angle);

  int x_line = std::round(ym * (point.y - y1) + x1);

  return (point.x > x_line);
}

struct BorderPointPartition {
  std::vector<Point> right, back, left;
};

/// Returns the distance between the two angles.
double angDist(double a, double b) {
  // special case
  if (b == -69)
    return 1e6;
  double diff = a - b;
  diff = std::fmod(std::fmod(diff, M_PI * 2) + M_PI * 2, M_PI * 2);
  if (diff < M_PI)
    return diff;
  else
    return M_PI * 2 - diff;
}

BorderPointPartition findBorderPoints(Frame &frame, const Pose &poseEstimate) {
  BorderPointPartition result;

  Line leftLine = Line::BORDER_OUT_1, backLine = Line::BORDER_OUT_2,
       rightLine = Line::BORDER_IN_1;
  if (projectLine(poseEstimate, Line::BORDER_OUT_4).has_value() &&
      projectLine(poseEstimate, Line::BORDER_OUT_1).has_value()) {
    leftLine = Line::BORDER_OUT_4;
    backLine = Line::BORDER_OUT_1;
    std::cout << "yes" << std::endl;
  } else
    for (auto potentialLeft : outerLines) {
      if (projectLine(poseEstimate, potentialLeft, false).has_value()) {
        leftLine = potentialLeft;
        backLine = Line((int(leftLine) + 1) % 4);
        break;
      }
    }
  if (projectLine(poseEstimate, Line::BORDER_IN_4).has_value() &&
      projectLine(poseEstimate, Line::BORDER_IN_1).has_value()) {
    rightLine = Line::BORDER_IN_4;
  } else
    for (auto potentialRight : innerLines) {
      if (projectLine(poseEstimate, potentialRight, false).has_value()) {
        rightLine = potentialRight;
        break;
      }
    }

  std::cout << "left is: " << int(leftLine) << ", back is: " << int(backLine)
            << "right is: " << int(rightLine) << std::endl;

  double angleLeft, angleBack, angleRight;

  auto projectedLeft = projectLine(poseEstimate, leftLine, false);
  if (projectedLeft.has_value()) {
    angleLeft = std::fmod(projectedLeft->angle + M_PI_2 + M_PI * 2, M_PI * 2);
  } else {
    angleLeft = -69;
  }

  auto projectedBack = projectLine(poseEstimate, backLine, false);
  if (projectedBack.has_value()) {
    angleBack = std::fmod(projectedBack->angle + M_PI_2 + M_PI * 2, M_PI * 2);
  } else {
    angleBack = -69;
  }

  auto projectedRight = projectLine(poseEstimate, rightLine, false);
  if (projectedRight.has_value()) {
    angleRight = std::fmod(projectedRight->angle - M_PI_2 + M_PI * 2, M_PI * 2);
  } else {
    angleRight = -69;
  }

  std::vector<Point> nonfits;

  for (int y = 0; y < HEIGHT; ++y) {
    for (int x = 0; x < WIDTH; ++x) {
      Point p{x, y};
      if (isBorderPoint(frame, p)) {
        double gradAngle = directionOfGradientAtPoint(p, frame);
        std::array<double, 4> angDists = {angDist(gradAngle, angleLeft),
                                          angDist(gradAngle, angleBack),
                                          angDist(gradAngle, angleRight), 0.3};
        switch (
            std::distance(angDists.begin(),
                          std::min_element(angDists.begin(), angDists.end()))) {
        case 0: {
          result.left.push_back(p);
          break;
        }
        case 1: {
          result.back.push_back(p);
          break;
        }
        case 2: {
          result.right.push_back(p);
          break;
        }
        case 3: {
          nonfits.push_back(p);
          break;
        }
        }
      }
    }
  }

  colorColor(frame, nonfits, {0, 0, 0});

  return result;
}

ScreenLineSet findLines(Frame &frame, const Pose &poseEstimate) {
  ScreenLineSet lines;

  BorderPointPartition borderPartition = findBorderPoints(frame, poseEstimate);

  lines.right = findLine(borderPartition.right);
  if (lines.right.has_value()) {
    std::vector<Point> nback;
    std::vector<Point> nleft;
    // they are sorted by y, i checked
    int rminy = borderPartition.right[borderPartition.right.size() / 10].y;
    for (const auto &p : borderPartition.back) {
      if (!isRight(*lines.right, p) || p.y < rminy - 100)
        nback.push_back(p);
    }
    for (const auto &p : borderPartition.left) {
      if (!isRight(*lines.right, p) || p.y < rminy - 100)
        nleft.push_back(p);
    }
    borderPartition.back = nback;
    borderPartition.left = nleft;
  }
  lines.left = findLine(borderPartition.left);
  lines.back = findLine(borderPartition.back);

  colorColor(frame, borderPartition.left, {113, 255, 255});
  colorColor(frame, borderPartition.back, {239, 255, 255});
  colorColor(frame, borderPartition.right, {64, 255, 255});

  std::vector<Point> orangePoints, bluePoints;
  for (int x = 0; x < WIDTH; ++x) {
    for (int y = 0; y < HEIGHT; ++y) {
      Point p{x, y};
      bool inField = true;
      if (lines.left.has_value())
        inField &= isRight(*lines.left, p);
      if (lines.back.has_value())
        inField &= isBelow(*lines.back, p);
      if (lines.right.has_value())
        inField &= !isRight(*lines.right, p);
      if (inField) {
        if (isOrange(frame.HSV[p.y * WIDTH + p.x]))
          orangePoints.push_back(p);
        if (isBlue(frame.HSV[p.y * WIDTH + p.x]))
          bluePoints.push_back(p);
      }
    }
  }

  colorColor(frame, orangePoints, {200, 255, 255});
  colorColor(frame, bluePoints, {200, 0, 255});

  lines.blue = findLine(bluePoints);
  lines.orange = findLine(orangePoints);

  return lines;
}

void drawScreenLineSet(Frame &frame, ScreenLineSet lines) {

  if (lines.left.has_value())
    drawLineInFrame(frame, *lines.left, {212, 255, 255});
  else
    std::cout << "Unfortunately, there was no left line." << std::endl;
  if (lines.back.has_value())
    drawLineInFrame(frame, *lines.back, {43, 255, 255});
  else
    std::cout << "Unfortunately, there was no back line." << std::endl;
  if (lines.right.has_value())
    drawLineInFrame(frame, *lines.right, {85, 255, 255});
  else
    std::cout << "Unfortunately, there was no right line." << std::endl;
  if (lines.orange.has_value())
    drawLineInFrame(frame, *lines.orange, {127, 255, 255});
  else
    std::cout << "Unfortunately, there was no orange line." << std::endl;
  if (lines.blue.has_value())
    drawLineInFrame(frame, *lines.blue, {0, 255, 255});
  else
    std::cout << "Unfortunately, there was no blue line." << std::endl;
}
