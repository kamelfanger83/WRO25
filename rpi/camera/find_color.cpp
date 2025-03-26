#include <iostream>
#include <vector>
#include <cmath>
#include <cstdint>

#include "camera.h"
#include "imagemask.h"


struct Segment
{
    /* data */
    Point start;
    Point end;
};



std::vector<Point> mask(const Frame& img, bool (*is_color)(uint8_t, uint8_t, uint8_t) ){
    // color array: [random, R, G, B]
    std::vector<Point> points;
    for (int x = 0; x < WIDTH; ++x) {
        for (int y = 0; y < HEIGHT; ++y) {
            int i = y * WIDTH + x;
            int r = img.XRGB[i * 4 + 1];
            int g = img.XRGB[i * 4 + 2];
            int b = img.XRGB[i * 4 + 3];
            if (is_color(r, g, b)){
                points.push_back({x, y});
            }
        }
    }
    return points;
}

Frame colorColor(const Frame& img, std::vector<Point> points){
    Frame colored_img = img;
    int r = 255;
    int g = 0;
    int b = 0;
    for (Point p : points){
        int i = p.y * WIDTH + p.x;
        colored_img.XRGB[i * 4 + 1] = 255;
        colored_img.XRGB[i * 4 + 2] = 0;
        colored_img.XRGB[i * 4 + 3] = 0;
    }
    return colored_img;
}




bool is_blue(uint8_t R, uint8_t G, uint8_t B){
    if (B > 150 && B /2 >= R && B /2 >= G){
        return true;
    }
    return false;
}

bool is_orange(uint8_t R, uint8_t G, uint8_t B){
    
    if (G /2 >= B && R >= G + 20 && R >= 200){
        return true;
    }
    return false;
}

bool is_black(uint8_t R, uint8_t G, uint8_t B){
    
    if (R + G + B <= 150){
        return true;
    }
    return false;
}
bool is_white(uint8_t R, uint8_t G, uint8_t B){
    
    if (R + G + B >= 600){
        return true;
    }
    return false;
}

bool is_green(uint8_t R, uint8_t G, uint8_t B){

    if (G > 150 && G /2 >= R && G /2 >= B){
        return true;
    }
    return false;
}

bool is_red(uint8_t R, uint8_t G, uint8_t B){
    
    if (R > 150 && R /2 >= G && R /2 >= B){
        return true;
    }
    return false;
}




int main(){

    Frame image;


    std::cout << "Hello World";
    return 0;
}