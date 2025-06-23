#pragma once

#include <cstdint>
#include <vector>
#include "DrmDevice.h"
#include "DisplayMode.h"

class DrmFramebuffer {
public:
    DrmFramebuffer(DrmDevice& device, const DisplayMode& mode);
    ~DrmFramebuffer();

    uint32_t getId() const;

    uint32_t* data();

private:
    int fd;
    uint32_t fb_id;
    uint32_t handle;
    uint32_t pitch;
    uint32_t size;
    uint8_t* map = nullptr;
    int drm_fd;
    int bo_fd;
};
