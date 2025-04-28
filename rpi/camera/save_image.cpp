#include <cmath>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "camera.h"

void hsvToRgb(const HSVPixel &hsv, uint8_t &r, uint8_t &g, uint8_t &b) {
  uint8_t region = hsv.h / 43;
  uint8_t remainder = (hsv.h - region * 43) * 6;

  uint8_t p = (hsv.v * (255 - hsv.s)) / 255;
  uint8_t q = (hsv.v * (255 - (hsv.s * remainder) / 255)) / 255;
  uint8_t t = (hsv.v * (255 - (hsv.s * (255 - remainder) / 255))) / 255;

  switch (region) {
  case 0:
    r = hsv.v;
    g = t;
    b = p;
    break;
  case 1:
    r = q;
    g = hsv.v;
    b = p;
    break;
  case 2:
    r = p;
    g = hsv.v;
    b = t;
    break;
  case 3:
    r = p;
    g = q;
    b = hsv.v;
    break;
  case 4:
    r = t;
    g = p;
    b = hsv.v;
    break;
  default:
    r = hsv.v;
    g = p;
    b = q;
    break;
  }
}

void convertHSVtoRGB(HSVPixel *src, uint8_t *dst, int width, int height) {
  for (int i = 0; i < width * height; i++) {
    hsvToRgb(src[i], dst[i * 3 + 0], dst[i * 3 + 1], dst[i * 3 + 2]);
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
  convertHSVtoRGB(frame.HSV, rgb_data, WIDTH, HEIGHT);

  std::string source_path =
      std::filesystem::canonical(__FILE__).parent_path().string();
  std::string output_path =
      /* source_path + "/" + */ std::to_string(frame.timestamp) + ".png";

  // Save the image as PNG
  saveImage(rgb_data, WIDTH, HEIGHT, output_path.c_str());

  std::cout << "We saved an image!" << std::endl;

  delete[] rgb_data;
}
