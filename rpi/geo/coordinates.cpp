#pragma once

#include <cassert>
#include <utility>

#include "../structs.h"

enum class Line {
  // outer walls
  BORDER_OUT_1,
  BORDER_OUT_2,
  BORDER_OUT_3,
  BORDER_OUT_4,
  // inner walls
  BORDER_IN_1,
  BORDER_IN_2,
  BORDER_IN_3,
  BORDER_IN_4,
  // orange lines
  ORANGE_1,
  ORANGE_2,
  ORANGE_3,
  ORANGE_4,
  // blue lines
  BLUE_1,
  BLUE_2,
  BLUE_3,
  BLUE_4,
  INVALID,
};

/// 4 arrays with all the important lines
std::array<Line, 4> outerLines = {Line::BORDER_OUT_1, Line::BORDER_OUT_2,
                                  Line::BORDER_OUT_3, Line::BORDER_OUT_4};
std::array<Line, 4> innerLines = {Line::BORDER_IN_1, Line::BORDER_IN_2,
                                  Line::BORDER_IN_3, Line::BORDER_IN_4};
std::array<Line, 4> orangeLines = {Line::ORANGE_1, Line::ORANGE_2,
                                   Line::ORANGE_3, Line::ORANGE_4};
std::array<Line, 4> blueLines = {Line::BLUE_1, Line::BLUE_2, Line::BLUE_3,
                                 Line::BLUE_4};

/// function that takes a line and returns the start and end Cordinates of said
/// line
std::pair<Vector, Vector> getStartEndPoints(Line line) {
  if (flipped) {
    switch (line) {
    case Line::BLUE_1: {
      line = Line::ORANGE_1;
      break;
    }
    case Line::BLUE_2: {
      line = Line::ORANGE_2;
      break;
    }
    case Line::BLUE_3: {
      line = Line::ORANGE_3;
      break;
    }
    case Line::BLUE_4: {
      line = Line::ORANGE_4;
      break;
    }
    case Line::ORANGE_1: {
      line = Line::BLUE_1;
      break;
    }
    case Line::ORANGE_2: {
      line = Line::BLUE_2;
      break;
    }
    case Line::ORANGE_3: {
      line = Line::BLUE_3;
      break;
    }
    case Line::ORANGE_4: {
      line = Line::BLUE_4;
      break;
    }
    default:
      break;
    }
  }
  switch (line) {
  // Coordinates outer walls
  case Line::BORDER_OUT_1:
    return {{0, 0, 0}, {0, 300, 0}};
  case Line::BORDER_OUT_2:
    return {{0, 300, 0}, {300, 300, 0}};
  case Line::BORDER_OUT_3:
    return {{300, 300, 0}, {300, 0, 0}};
  case Line::BORDER_OUT_4:
    return {{300, 0, 0}, {0, 0, 0}};

  // Coordinates inner walls
  case Line::BORDER_IN_1:
    return {{100, 100, 0}, {100, 200, 0}};
  case Line::BORDER_IN_2:
    return {{100, 200, 0}, {200, 200, 0}};
  case Line::BORDER_IN_3:
    return {{200, 200, 0}, {200, 100, 0}};
  case Line::BORDER_IN_4:
    return {{200, 100, 0}, {100, 100, 0}};

  // Coordinates orange lines, from inner wall to outer wall
  case Line::ORANGE_1:
    return {{100, 98, 0}, {42, 0, 0}};
  case Line::ORANGE_2:
    return {{98, 200, 0}, {0, 258, 0}};
  case Line::ORANGE_3:
    return {{200, 202, 0}, {258, 300, 0}};
  case Line::ORANGE_4:
    return {{202, 100, 0}, {300, 42, 0}};

  // Coordinates blue lines from inner to outer wall
  case Line::BLUE_1:
    return {{98, 100, 0}, {0, 42, 0}};
  case Line::BLUE_2:
    return {{100, 202, 0}, {42, 300, 0}};
  case Line::BLUE_3:
    return {{202, 200, 0}, {300, 258, 0}};
  case Line::BLUE_4:
    return {{200, 98, 0}, {258, 0, 0}};

  default:
    return {{-1, -1, -1}, {-1, -1, -1}}; // Error case
  }
}

enum Segment { SEGMENT_1, SEGMENT_2, SEGMENT_3, SEGMENT_4 };

Segment inSegment(Pose pose) {
  double x = pose.x;
  double y = pose.y;

  if (x >= 0 && x < 100 && y >= 100 && y <= 300) {
    return SEGMENT_1;
  }
  if (x > 100 && x <= 300 && y >= 200 && y <= 300) {
    return SEGMENT_2;
  }
  if (x > 200 && x <= 300 && y >= 0 && y <= 200) {
    return SEGMENT_3;
  }
  if (x >= 0 && x <= 200 && y >= 0 && y < 100) {
    return SEGMENT_4;
  }
  assert(false);
  return SEGMENT_1;
}

/// takes a 3d Position which we consider to be in the first Segment
//  and a Segment in which the position gets transformed intos
Vector vectorInSegment(Segment seg, Vector poseInFirst) {
  Vector result;
  if (seg == Segment::SEGMENT_1) {
    result = poseInFirst;
  }
  if (seg == Segment::SEGMENT_2) {
    result.x = poseInFirst.y;
    result.y = 300 - poseInFirst.x;
    result.z = 0;
  }
  if (seg == Segment::SEGMENT_3) {
    result.x = 300 - poseInFirst.x;
    result.y = 300 - poseInFirst.y;
    result.z = 0;
  }
  if (seg == Segment::SEGMENT_4) {
    result.x = 300 - poseInFirst.y;
    result.y = poseInFirst.x;
    result.z = 0;
  }

  return result;
}


Waypoint wayPointInSegment(Segment s, Waypoint w){
  Vector v(w.x, w.y, 0.0);
  v = vectorInSegment(s, v);

  w.x = v.x;
  w.y = v.y;
  return w;
}

Segment inSegmentSlanted(Pose pose) {
  double x = pose.x;
  double y = pose.y;

  if (x >= 0 && x < 100 && y >= x && y <= 300 - x) {
    return SEGMENT_1;
  }
  if (x > 300 - y && x <= y && y >= 200 && y <= 300) {
    return SEGMENT_2;
  }
  if (x >= 200 && x <= 300 && y >= 300 - x && x >= y) {
    return SEGMENT_3;
  }
  if (x >= y && x <= 300 - y && y >= 0 && y < 100) {
    return SEGMENT_4;
  }
  assert(false);
  return SEGMENT_1;
}


Segment nextSegment(Segment s){
  if(s == Segment::SEGMENT_1){
    return Segment::SEGMENT_2;
  }
  if(s == Segment::SEGMENT_2){
    return Segment::SEGMENT_3;
  }
  if(s == Segment::SEGMENT_3){
    return Segment::SEGMENT_4;
  }
  if(s == Segment::SEGMENT_4){
    return Segment::SEGMENT_1;
  }
}