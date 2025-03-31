#pragma once

#include <cstdint>

#define WIDTH 800
#define HEIGHT 600

struct Frame {
  uint8_t *XRGB;
  long long timestamp;
};

extern Frame lastFrame;

/// Sets up the camera to start capturing frames, also queues first frame to be
/// captured.
void initializeCamera();

/// Queues another frame to be captured.
void queueCapture();

/// Frees all resources allocated when initializing the camera.
void cleanCamera();
