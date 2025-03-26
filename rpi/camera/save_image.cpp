#include <cmath>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "camera.h"

void convertXRGBtoRGB(const uint8_t *src, uint8_t *dst, int width, int height) {
  int pixel_count = width * height;
  for (int i = 0; i < pixel_count; i++) {
    dst[i * 3 + 0] = src[i * 4 + 1]; // Red
    dst[i * 3 + 1] = src[i * 4 + 2]; // Green
    dst[i * 3 + 2] = src[i * 4 + 3]; // Blue
  }
}

void saveImage(const uint8_t *rgb_data, int width, int height,
               const char *filename) {
  stbi_write_png(filename, width, height, 3, rgb_data, width * 3);
}

/// Saves the frame to "{timestamp}.png"
void saveFrame(const Frame &frame) {

  // Allocate RGB buffer
  uint8_t *rgb_data = new uint8_t[WIDTH * HEIGHT * 3];

  // Convert XRGB to RGB
  convertXRGBtoRGB(frame.XRGB, rgb_data, WIDTH, HEIGHT);

  std::string source_path =
      std::filesystem::canonical(__FILE__).parent_path().string();
  std::string output_path =
      source_path + "/" + std::to_string(frame.timestamp) + ".png";

  // Save the image as PNG
  saveImage(rgb_data, WIDTH, HEIGHT, output_path.c_str());

  std::cout << "We saved an image!" << std::endl;

  delete[] rgb_data;
}
