#pragma once
#include <cstdint>
#include <xf86drm.h>

class DrmFramebuffer {
public:
    int fd;
    uint32_t width;
    uint32_t height;
    uint32_t handle;
    uint32_t pitch;
    uint32_t size;
    uint32_t fb_id;
    void* map;

    DrmFramebuffer(int drm_fd, uint32_t w, uint32_t h);
    ~DrmFramebuffer();

    uint32_t* data() const;
};
