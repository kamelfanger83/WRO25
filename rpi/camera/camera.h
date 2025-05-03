#pragma once

#include <cstdint>

#include "../structs.h"

extern Frame lastFrame;

/// Sets up the camera to start capturing frames.
void initializeCamera();

/// Queues another frame to be captured.
void queueCapture();

/// Frees all resources allocated when initializing the camera.
void cleanCamera();

/// Returns a separate copy of a frame;
Frame cloneFrame(const Frame &);
