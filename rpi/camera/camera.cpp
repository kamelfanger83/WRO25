#include <algorithm>
#include <cstdint>

#include "../structs.h"
#include "camera.h"

HSVPixel rgbToHsv(uint8_t r, uint8_t g, uint8_t b) {
  uint8_t maxVal = std::max({r, g, b});
  uint8_t minVal = std::min({r, g, b});
  uint8_t delta = maxVal - minVal;

  uint8_t h = 0;
  if (delta != 0) {
    if (maxVal == r) {
      h = 43 * (g - b) / delta;
    } else if (maxVal == g) {
      h = 85 + 43 * (b - r) / delta;
    } else {
      h = 171 + 43 * (r - g) / delta;
    }
  }

  uint8_t s = maxVal == 0 ? 0 : (255 * delta) / maxVal;
  uint8_t v = maxVal;

  return HSVPixel{h, s, v};
}

Frame lastFrame{new HSVPixel[WIDTH * HEIGHT * 3], -1, {0, 0, 0}};

#ifndef USE_LIBCAMERA

#include <cassert>
#include <filesystem>
#include <iostream>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int frameIndex = 0;

void initializeCamera() { queueCapture(); }

void queueCapture() {
  std::string source_path =
      std::filesystem::canonical(__FILE__).parent_path().string();
  int x, y, channels;
  unsigned char *data = stbi_load(
      (source_path + "/frames/" + std::to_string(frameIndex) + ".png").c_str(),
      &x, &y, &channels, 0);
  if (x != WIDTH || y != HEIGHT) {
    std::cerr << "Expected an image with dimensions " << WIDTH << "x" << HEIGHT
              << ". Got " << x << "x" << y << "instead.";
    assert(false);
  }
  for (int i = 0; i < WIDTH * HEIGHT; ++i) {
    lastFrame.HSV[i] = rgbToHsv(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
  }
  stbi_image_free(data);
  lastFrame.timestamp = frameIndex;
  std::cout << "Read frame #" << frameIndex << std::endl;
  ++frameIndex;
}

void cleanCamera() { delete[] lastFrame.HSV; }

#else
/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2020, Ideas on Board Oy.
 *
 * A simple libcamera capture example
 */

#include <cassert>
#include <iostream>
#include <string>
#include <sys/mman.h>

#include <libcamera/libcamera.h>

#include "../serial.cpp"
#include "../utils.cpp"

using namespace libcamera;
static std::shared_ptr<Camera> camera;

Stream *stream;
std::unique_ptr<Request> request;
FrameBufferAllocator *allocator;
std::unique_ptr<CameraManager> cm;

Pose lastArduinoPose{0, 0, 0};

static void requestComplete(Request *request) {

  processArduinoResponse();

  lastFrame.relativeArduinoPose = arduinoPose / lastArduinoPose;
  lastArduinoPose = arduinoPose;

  std::cout << std::endl
            << "Request completed: " << request->toString() << std::endl;

  FrameBuffer *buffer = request->findBuffer(stream);
  const FrameMetadata &metadata = buffer->metadata();

  unsigned int nplane = 0;
  for (const FrameMetadata::Plane &plane : metadata.planes()) {
    std::cout << plane.bytesused;
    if (++nplane < metadata.planes().size())
      std::cout << "/";
  }

  const auto &plane = buffer->planes()[0];
  size_t length = plane.length;
  size_t offset = plane.offset;

  // Memory map the buffer
  uint8_t *XRGB =
      static_cast<uint8_t *>(mmap(nullptr, length, PROT_READ | PROT_WRITE,
                                  MAP_SHARED, plane.fd.get(), offset));

  for (int i = 0; i < WIDTH * HEIGHT; ++i) {
    lastFrame.HSV[i] = rgbToHsv(XRGB[i * 4 + 2], XRGB[i * 4 + 1], XRGB[i * 4]);
  }

  // I can't be bothered to do this properly
  lastFrame.timestamp = std::stoll(request->metadata().get(45).toString());
}

void queueCapture() {
  /* Re-queue the Request to the camera. */
  request->reuse(Request::ReuseBuffers);
  camera->queueRequest(request.get());
}

void initializeCamera() {
  cm = std::make_unique<CameraManager>();
  cm->start();

  std::string cameraId = cm->cameras()[0]->id();
  assert(cameraId == "/base/axi/pcie@120000/rp1/i2c@88000/imx708@1a");

  camera = cm->get(cameraId);
  camera->acquire();

  // Create configuration
  std::unique_ptr<CameraConfiguration> config =
      camera->generateConfiguration({StreamRole::Viewfinder});

  StreamConfiguration &streamConfig = config->at(0);
  std::cout << "Default viewfinder configuration is: "
            << streamConfig.toString() << std::endl;

  // streamConfig.pixelFormat = libcamera::formats::YUV420;

  config->validate();
  std::cout << "Validated viewfinder configuration is: "
            << streamConfig.toString() << std::endl;

  camera->configure(config.get());

  allocator = new FrameBufferAllocator(camera);

  for (StreamConfiguration &cfg : *config) {
    int ret = allocator->allocate(cfg.stream());
    if (ret < 0) {
      std::cerr << "Can't allocate buffers" << std::endl;
      return;
    }

    size_t allocated = allocator->buffers(cfg.stream()).size();
    std::cout << "Allocated " << allocated << " buffers for stream"
              << std::endl;
  }

  stream = streamConfig.stream();
  const std::vector<std::unique_ptr<FrameBuffer>> &buffers =
      allocator->buffers(stream);

  request = camera->createRequest();
  if (!request) {
    std::cerr << "Can't create request" << std::endl;
    return;
  }

  const std::unique_ptr<FrameBuffer> &buffer = buffers[0];
  std::cout << "Buffer with " << buffer->planes().size() << " planes"
            << std::endl;
  for (size_t i = 0; i < buffer->planes().size(); i++) {
    std::cout << "Plane " << i << " length: " << buffer->planes()[i].length
              << std::endl;
  }
  int ret = request->addBuffer(stream, buffer.get());
  if (ret < 0) {
    std::cerr << "Can't set buffer for request" << std::endl;
    return;
  }

  /*
   * Controls can be added to a request on a per frame basis.
   */
  // ControlList &controls = request->controls();
  // controls.set(controls::Brightness, 0.5);

  camera->requestCompleted.connect(requestComplete);

  camera->start();
  camera->queueRequest(request.get());
}

void cleanCamera() {
  camera->stop();
  allocator->free(stream);
  delete allocator;
  camera->release();
  camera.reset();
  cm->stop();

  delete[] lastFrame.HSV;

  return;
}
#endif
