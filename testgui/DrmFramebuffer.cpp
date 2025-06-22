#include "DrmFramebuffer.h"
#include <sys/mman.h>
#include <drm/drm.h>
#include <sys/ioctl.h>
#include <stdexcept>
#include <iostream>
#include <xf86drmMode.h>


DrmFramebuffer::DrmFramebuffer(int drm_fd, uint32_t w, uint32_t h)
    : fd(drm_fd), width(w), height(h), map(nullptr) {

    drm_mode_create_dumb create = {};
    create.width = width;
    create.height = height;
    create.bpp = 32;

    if (ioctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &create) < 0) {
        throw std::runtime_error("Failed to create dumb buffer");
    }

    handle = create.handle;
    pitch = create.pitch;
    size = create.size;

    if (drmModeAddFB(fd, width, height, 24, 32, pitch, handle, &fb_id)) {
        throw std::runtime_error("Failed to add framebuffer");
    }

    drm_mode_map_dumb map_dumb = {};
    map_dumb.handle = handle;

    if (ioctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &map_dumb) < 0) {
        throw std::runtime_error("Failed to map dumb buffer");
    }

    map = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, map_dumb.offset);
    if (map == MAP_FAILED) {
        throw std::runtime_error("mmap failed");
    }
}

DrmFramebuffer::~DrmFramebuffer() {
    if (map) munmap(map, size);
    drmModeRmFB(fd, fb_id);

    drm_mode_destroy_dumb destroy = {};
    destroy.handle = handle;
    ioctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy);
}

uint32_t* DrmFramebuffer::data() const {
    return static_cast<uint32_t*>(map);
}
