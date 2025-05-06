#include <algorithm>
#include <cstdint>
#include <cstring>

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

Frame lastFrame{new HSVPixel[WIDTH * HEIGHT * 3], -1};

Frame cloneFrame(const Frame &frame) {
  auto hsv = new HSVPixel[WIDTH * HEIGHT * 3];
  std::memcpy(hsv, frame.HSV, WIDTH * HEIGHT * 3);
  return {hsv, frame.timestamp};
}

#ifndef USE_LIBCAMERA

#include <cassert>
#include <filesystem>
#include <iostream>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int frameIndex = 0;

void initializeCamera() {}

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

static std::shared_ptr<libcamera::Camera> camera;

libcamera::Stream *stream;
std::unique_ptr<libcamera::Request> request;
libcamera::FrameBufferAllocator *allocator;
std::unique_ptr<libcamera::CameraManager> cm;
bool wantFrame = false;

static void requestComplete(libcamera::Request *request) {
  if (wantFrame) {
    wantFrame = false;
    std::cout << std::endl
              << "Request completed: " << request->toString() << std::endl;

    libcamera::FrameBuffer *buffer = request->findBuffer(stream);
    const libcamera::FrameMetadata &metadata = buffer->metadata();

    unsigned int nplane = 0;
    for (const libcamera::FrameMetadata::Plane &plane : metadata.planes()) {
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

    for (int y = 0; y < HEIGHT; ++y) {
      for (int x = 0; x < WIDTH; ++x) {
        int i;
        if (flipped)
          i = (y * WIDTH + (WIDTH - 1 - x)) * 4;
        else
          i = (y * WIDTH + x) * 4;
        lastFrame.HSV[y * WIDTH + x] =
            rgbToHsv(XRGB[i + 2], XRGB[i + 1], XRGB[i]);
      }
    }

    munmap(XRGB, length);

    // I can't be bothered to do this properly
    lastFrame.timestamp = std::stoll(request->metadata().get(45).toString());
  }

  /* Re-queue the Request to the camera. */
  request->reuse(libcamera::Request::ReuseBuffers);
  camera->queueRequest(request);
}

void queueCapture() { wantFrame = true; }

void initializeCamera() {
  cm = std::make_unique<libcamera::CameraManager>();
  cm->start();

  std::string cameraId = cm->cameras()[0]->id();
  assert(cameraId == "/base/axi/pcie@120000/rp1/i2c@88000/imx708@1a");

  camera = cm->get(cameraId);
  camera->acquire();

  // Create configuration
  std::unique_ptr<libcamera::CameraConfiguration> config =
      camera->generateConfiguration({libcamera::StreamRole::VideoRecording});

  libcamera::StreamConfiguration &streamConfig = config->at(0);
  std::cout << "Default viewfinder configuration is: "
            << streamConfig.toString() << std::endl;

  streamConfig.pixelFormat = libcamera::formats::XRGB8888;

  streamConfig.size.width = WIDTH;
  streamConfig.size.height = HEIGHT;

  config->validate();
  std::cout << "Validated viewfinder configuration is: "
            << streamConfig.toString() << std::endl;

  camera->configure(config.get());

  allocator = new libcamera::FrameBufferAllocator(camera);

  for (libcamera::StreamConfiguration &cfg : *config) {
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
  const std::vector<std::unique_ptr<libcamera::FrameBuffer>> &buffers =
      allocator->buffers(stream);

  request = camera->createRequest();
  if (!request) {
    std::cerr << "Can't create request" << std::endl;
    return;
  }

  const std::unique_ptr<libcamera::FrameBuffer> &buffer = buffers[0];
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
  // libcamera::ControlList &controls = request->controls();
  // controls.set(libcamera::controls::Brightness, 0.5);

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
