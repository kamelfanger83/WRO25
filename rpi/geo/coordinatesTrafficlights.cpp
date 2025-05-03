#include <vector>

#include "../structs.h"

// tl enumerated from front to back, left to right.
enum TrafficLight {
    TRAFFICLIGHT_1,
    TRAFFICLIGHT_2,
    TRAFFICLIGHT_3,
    TRAFFICLIGHT_4,
    TRAFFICLIGHT_5,
    TRAFFICLIGHT_6
};


std::vector<Vector> getTrafficLightCoordinates(TrafficLight tl) {
    switch (tl) {
        case TrafficLight::TRAFFICLIGHT_1:
            return{{ 37.5 , 97.5 , 0},
            { 37.5 , 102.5 , 0},
            { 42.5 , 97.5 , 0},
            { 42.5 , 102.5 , 0},
            { 37.5 , 97.5 , 10},
            { 37.5 , 102.5 , 10},
            { 42.5 , 97.5 , 10},
            { 42.5 , 102.5 , 10}};


        case TrafficLight::TRAFFICLIGHT_2:
            return{{ 57.5 , 97.5 , 0},
            { 57.5 , 102.5 , 0},
            { 62.5 , 97.5 , 0},
            { 62.5 , 102.5 , 0},
            { 57.5 , 97.5 , 10},
            { 57.5 , 102.5 , 10},
            { 62.5 , 97.5 , 10},
            { 62.5 , 102.5 , 10}};


        case TrafficLight::TRAFFICLIGHT_3:
            return{{ 37.5 , 147.5 , 0},
            { 37.5 , 152.5 , 0},
            { 42.5 , 147.5 , 0},
            { 42.5 , 152.5 , 0},
            { 37.5 , 147.5 , 10},
            { 37.5 , 152.5 , 10},
            { 42.5 , 147.5 , 10},
            { 42.5 , 152.5 , 10}};


        case TrafficLight::TRAFFICLIGHT_4:
            return{{ 57.5 , 147.5 , 0},
            { 57.5 , 152.5 , 0},
            { 62.5 , 147.5 , 0},
            { 62.5 , 152.5 , 0},
            { 57.5 , 147.5 , 10},
            { 57.5 , 152.5 , 10},
            { 62.5 , 147.5 , 10},
            { 62.5 , 152.5 , 10}};


        case TrafficLight::TRAFFICLIGHT_5:
            return{{ 37.5 , 197.5 , 0},
            { 37.5 , 202.5 , 0},
            { 42.5 , 197.5 , 0},
            { 42.5 , 202.5 , 0},
            { 37.5 , 197.5 , 10},
            { 37.5 , 202.5 , 10},
            { 42.5 , 197.5 , 10},
            { 42.5 , 202.5 , 10}};


        case TrafficLight::TRAFFICLIGHT_6:
            return{{ 57.5 , 197.5 , 0},
            { 57.5 , 202.5 , 0},
            { 62.5 , 197.5 , 0},
            { 62.5 , 202.5 , 0},
            { 57.5 , 197.5 , 10},
            { 57.5 , 202.5 , 10},
            { 62.5 , 197.5 , 10},
            { 62.5 , 202.5 , 10}};

    }
}

       