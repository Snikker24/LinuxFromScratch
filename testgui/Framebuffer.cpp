#include "DrmFramebuffer.h"
#include <iostream>
#include <cstring>
#include <linux/drm.h>
#include <sys/ioctl.h>

DrmFramebuffer::DrmFramebuffer(int fd, uint32_t width, uint32_t height, uint32_t bpp)
    : fd(fd), width(width), height(height), bpp(bpp),
      pitch(0), handle(0), fb_id(0), size(0), map_ptr(nullptr)
{
    struct drm_mode_create_dumb create_dumb = {};
    create_dumb.width = width;
    create_dumb.height = height;
    create_dumb.bpp = bpp;

    if (ioctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &create_dumb) < 0) {
        throw std::runtime_error("Failed to create dumb buffer");
    }

    pitch = create_dumb.pitch;
    size = create_dumb.size;
    handle = create_dumb.handle;

    if (drmModeAddFB(fd, width, height, 24, bpp, pitch, handle, &fb_id)) {
        // Clean up buffer
        struct drm_mode_destroy_dumb destroy = {};
        destroy.handle = handle;
        ioctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy);
        throw std::runtime_error("Failed to add framebuffer");
    }

    struct drm_mode_map_dumb map_dumb = {};
    map_dumb.handle = handle;
    if (ioctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &map_dumb) < 0) {
        drmModeRmFB(fd, fb_id);
        struct drm_mode_destroy_dumb destroy = {};
        destroy.handle = handle;
        ioctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy);
        throw std::runtime_error("Failed to map dumb buffer");
    }

    map_ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, map_dumb.offset);
    if (map_ptr == MAP_FAILED) {
        drmModeRmFB(fd, fb_id);
        struct drm_mode_destroy_dumb destroy = {};
        destroy.handle = handle;
        ioctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy);
        throw std::runtime_error("Failed to mmap framebuffer");
    }

    // Clear framebuffer memory
    std::memset(map_ptr, 0, size);
}

DrmFramebuffer::~DrmFramebuffer() {
    if (map_ptr && map_ptr != MAP_FAILED) {
        munmap(map_ptr, size);
        map_ptr = nullptr;
    }

    if (fb_id) {
        drmModeRmFB(fd, fb_id);
        fb_id = 0;
    }

    if (handle) {
        struct drm_mode_destroy_dumb destroy = {};
        destroy.handle = handle;
        ioctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy);
        handle = 0;
    }
}

DrmFramebuffer::DrmFramebuffer(DrmFramebuffer&& other) noexcept
    : fd(other.fd), width(other.width), height(other.height), bpp(other.bpp),
      pitch(other.pitch), handle(other.handle), fb_id(other.fb_id), size(other.size),
      map_ptr(other.map_ptr)
{
    other.map_ptr = nullptr;
    other.fb_id = 0;
    other.handle = 0;
}

DrmFramebuffer& DrmFramebuffer::operator=(DrmFramebuffer&& other) noexcept {
    if (this != &other) {
        if (map_ptr && map_ptr != MAP_FAILED) {
            munmap(map_ptr, size);
        }
        if (fb_id) {
            drmModeRmFB(fd, fb_id);
        }
        if (handle) {
            struct drm_mode_destroy_dumb destroy = {};
            destroy.handle = handle;
            ioctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy);
        }

        fd = other.fd;
        width = other.width;
        height = other.height;
        bpp = other.bpp;
        pitch = other.pitch;
        handle = other.handle;
        fb_id = other.fb_id;
        size = other.size;
        map_ptr = other.map_ptr;

        other.map_ptr = nullptr;
        other.fb_id = 0;
        other.handle = 0;
    }
    return *this;
}
