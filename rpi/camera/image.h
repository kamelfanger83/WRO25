#include <cstdint>

#define WIDTH 800
#define HEIGHT 600


struct Image {
    uint8_t* XRGB;
    long long timestamp;
};