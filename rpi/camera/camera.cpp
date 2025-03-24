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

#include "camera.h"

using namespace libcamera;
static std::shared_ptr<Camera> camera;

Stream *stream;
std::unique_ptr<Request> request;
FrameBufferAllocator *allocator;
std::unique_ptr<CameraManager> cm;

Frame lastFrame{nullptr, -1};

static void requestComplete(Request *request) {
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
  lastFrame.XRGB =
      static_cast<uint8_t *>(mmap(nullptr, length, PROT_READ | PROT_WRITE,
                                  MAP_SHARED, plane.fd.get(), offset));
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
  ControlList &controls = request->controls();
  controls.set(controls::Brightness, 0.5);

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

  return;
}
