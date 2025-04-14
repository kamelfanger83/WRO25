#pragma once

#include <cstdint>

#include "../structs.h"

#define WIDTH 800
#define HEIGHT 600

extern Frame lastFrame;

/// Sets up the camera to start capturing frames, also queues first frame to be
/// captured.
void initializeCamera();

/// Queues another frame to be captured.
void queueCapture();

/// Frees all resources allocated when initializing the camera.
void cleanCamera();
