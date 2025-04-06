#include <utility>

#include "../structs.h"

enum Line {
    //outer walls
    BORDER_OUT_1, BORDER_OUT_2, BORDER_OUT_3, BORDER_OUT_4,
    //inner walls
    BORDER_IN_1, BORDER_IN_2, BORDER_IN_3,BORDER_IN_4,
    //orange lines
    ORANGE_1, ORANGE_2, ORANGE_3, ORANGE_4,
    //blue lines
    BLUE_1, BLUE_2, BLUE_3, BLUE_4,
};


/// 4 arrays with all the important lines
Line outerLines[4] = {Line::BORDER_OUT_1, Line::BORDER_OUT_2, Line::BORDER_OUT_3, Line::BORDER_OUT_4};
Line innerLines[4] = {Line::BORDER_IN_1, Line::BORDER_IN_2, Line::BORDER_IN_3, Line::BORDER_IN_4};
Line orangeLines[4] = {Line::ORANGE_1, Line::ORANGE_2, Line::ORANGE_3, Line::ORANGE_4};
Line blueLines[4] = {Line::BLUE_1, Line::BLUE_2, Line::BLUE_3, Line::BLUE_4};

/// function that takes a line and returns the start and end Cordinates of said line
std::pair<Vector, Vector> getStartEndPoints(Line line) {
    switch (line) {
        //Coordinates outer walls
        case Line::BORDER_OUT_1: return {{0, 0, 0}, {0, 300, 0}};
        case Line::BORDER_OUT_2: return {{0, 300, 0}, {300, 300, 0}};
        case Line::BORDER_OUT_3: return {{300, 300, 0}, {300, 0, 0}};
        case Line::BORDER_OUT_4: return {{300, 0, 0}, {0, 0, 0}};

        //Coordinates inner walls
        case Line::BORDER_IN_1: return {{100, 100, 0}, {100, 200, 0}};
        case Line::BORDER_IN_2: return {{100, 200, 0}, {200, 200, 0}};
        case Line::BORDER_IN_3: return {{200, 200, 0}, {200, 100, 0}};
        case Line::BORDER_IN_4: return {{200, 100, 0}, {100, 100, 0}};

        //Coordinates orange lines, from inner wall to outer wall
        case Line::ORANGE_1: return {{100, 98, 0}, {42, 0, 0}};
        case Line::ORANGE_2: return {{98, 200, 0}, {0, 258, 0}};
        case Line::ORANGE_3: return {{200, 202, 0}, {258, 300, 0}};
        case Line::ORANGE_4: return {{202, 100, 0}, {300, 42, 0}};

        //Coordinates blue lines from inner to outer wall
        case Line::BLUE_1: return {{98, 100, 0}, {0, 42, 0}};
        case Line::BLUE_2: return {{100, 202, 0}, {42, 300, 0}};
        case Line::BLUE_3: return {{202, 200, 0}, {300, 258, 0}};
        case Line::BLUE_4: return {{200, 98, 0 }, {258, 0, 0}};

        default: return {{-1, -1, -1}, {-1, -1, -1}}; // Error case
    }
}
