#include <iostream>
#include <vector>
#include <cmath>
#include <cstdint>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


#include <fstream>
#include <cstdint>

#include <filesystem>

struct Image {
    uint8_t* data;  // Pointer to XRGB pixel data
    int width;
    int height;
};


void convertXRGBtoRGB(const uint8_t* src, uint8_t* dst, int width, int height) {
    int pixel_count = width * height;
    for (int i = 0; i < pixel_count; i++) {
        dst[i * 3 + 0] = src[i * 4 + 1];  // Red
        dst[i * 3 + 1] = src[i * 4 + 2];  // Green
        dst[i * 3 + 2] = src[i * 4 + 3];  // Blue
    }
}

void saveImage(const uint8_t* rgb_data, int width, int height, const char* filename) {
    stbi_write_png(filename, width, height, 3, rgb_data, width * 3);
}


int main() {
    const int width = 2, height = 2;
    Image img;
    img.width = width;
    img.height = height;


    
    
    // Assume data is filled somehow (e.g., loaded from a file)
    img.data = new uint8_t[width * height * 4]; 

    
    // Allocate RGB buffer
    uint8_t* rgb_data = new uint8_t[width * height * 3];
    
    // Convert XRGB to RGB
    convertXRGBtoRGB(img.data, rgb_data, width, height);
    
    //std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;
    std::string source_path = std::filesystem::canonical(__FILE__).parent_path().string();
    std::string output_path = source_path + "/output_image.png";

    // Save the image as PNG
    saveImage(rgb_data, width, height, output_path.c_str());
    
    // Clean up memory
    delete[] img.data;
    delete[] rgb_data;

    std::cout << "Image saved as 'output_image.png'!" << std::endl;
    return 0;
}
