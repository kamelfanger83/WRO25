#include <iostream>
#include "../../structs.h"
#include "find_color.cpp"
#include "find_line.cpp"

ScreenLineSet analyseFrame(const Frame &frame) {
    ScreenLineSet lines;

    unsigned int threshold = 250; // number of points needed to say that there is a line to be analysed
    auto bluepoints = mask(frame, isBlue);
    auto orangepoints = mask(frame, isOrange);

    // stores colored line iff it ∃ (= enough points of that color).
    if (bluepoints.size() >= threshold) lines.blue = findLine(bluepoints);
    if (orangepoints.size() >= threshold) lines.orange = findLine(orangepoints);

/*
Idea how to treat gradient:
make a threshold for the magnitude of the gradient to consider to be a border of the field.

*/



  return lines;
}