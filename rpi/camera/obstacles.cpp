#include <optional>

#include "../geo/camera.cpp"
#include "../structs.h"
#include "draw_line.cpp"
#include "find_color.cpp"
/// looks if there is a traffic light and if yes, if it is red or green

/// PRE: frame, pose, unprojected points of the suspected traffic lights
/// POST: returns 'r' if red, 'g' if green, std::nullopt if no traffic light is
/// found
std::optional<char>
checkTrafficLight(Frame &frame, const CoordinateSystem &cameraSystem,
                  const std::vector<Vector> &unprojectedPoints) {
  // projects every point on the screen and stores it into "points" iff on the
  // screen (= in front of the camera).
  std::vector<ScreenPosition> points = {};

  for (Vector unprojectedPoint : unprojectedPoints) {
    auto point = projectPoint(cameraSystem, unprojectedPoint);
    if (point.has_value())
      points.push_back(*point);
  }

  if (points.size() < 8)
    return std::nullopt; // no point on the screen
  int maxx = points[0].x, maxy = points[0].y, minx = points[0].x,
      miny = points[0].y;
  for (ScreenPosition point : points) {
    if (point.x > maxx)
      maxx = point.x;
    if (point.y > maxy)
      maxy = point.y;
    if (point.x < minx)
      minx = point.x;
    if (point.y < miny)
      miny = point.y;
  }

  minx = std::max(minx, 0); // adjust minx, miny, maxx, maxy to be in the frame
  miny = std::max(miny, 0);
  maxx = std::min(maxx, WIDTH - 1);
  maxy = std::min(maxy, HEIGHT - 1);

  int totalPixels = (maxx - minx) * (maxy - miny);
  int redPixels = 0, greenPixels = 0;

  for (int i = minx; i < maxx; ++i) {
    for (int j = miny; j < maxy; ++j) {
      if (isRed(frame.HSV[j * WIDTH + i]))
        ++redPixels; // red light
      if (isGreen(frame.HSV[j * WIDTH + i]))
        ++greenPixels; // green light
    }
  }

  // CHECK IF totalPixels is enough to say that it is on the frame.
  drawTrafficLights(frame, minx + 1, maxx - 1, miny + 1, maxy - 1,
                    {}); // draw the box around the traffic light

  double Threshold = 0.2; // 50% of the pixels have to be red or green to be
                          // considered a traffic light
  if (redPixels > greenPixels && redPixels / double(totalPixels) > Threshold) {

    drawTrafficLights(frame, minx, maxx, miny, maxy,
                      {255, 255, 255}); // draw the box around the traffic light
    return 'r';                         // red light
  }
  if (greenPixels / double(totalPixels) > Threshold) {
    drawTrafficLights(
        frame, minx, maxx, miny, maxy,
        {255 / 3, 255, 255}); // draw the box around the traffic light
    return 'g';               // green light
  }

  return std::nullopt; // no traffic light found
}
