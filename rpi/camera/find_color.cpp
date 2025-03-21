#include <iostream>
#include <vector>
#include <cmath>
#include <cstdint>

#include "image.h"
#include "imagemask.h"

bool createMask(const Image& img, const ImageMask& mask){
    
}

bool is_color(std::vector<int> pxl, std::vector<int> color, int T){
    
    int deviation = 0;
    
    for (int i = 0; i != 3; ++i){
        deviation += std::pow((pxl[i] - color[i]), 2);
    }
    
    deviation = std::sqrt(deviation);
    
    return (deviation < T);
    
}


int main(){
    
    int witdh;
    int height;

    Image image;
    ImageMask mask;


    std::cout << "Hello World";
    return 0;
}