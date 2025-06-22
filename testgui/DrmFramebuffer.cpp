#include "DrmFramebuffer.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdexcept>
#include <cstring>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm/drm.h>
#include <drm/drm_mode.h>

DrmFramebuffer::DrmFramebuffer(DrmDevice& device, const DisplayMode& mode)
    : fd(device.fd)
{
    uint32_t width = mode.getWidth();
    uint32_t height = mode.getHeight();
    pitch = width * 4;
    size = pitch * height;

    struct drm_mode_create_dumb create = {};
    create.width = width;
    create.height = height;
    create.bpp = 32;

    if (drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &create) < 0) {
        throw std::runtime_error("Failed to create dumb buffer");
    }

    handle = create.handle;
    pitch = create.pitch;
    size = create.size;

    if (drmModeAddFB(fd, width, height, 24, 32, pitch, handle, &fb_id)) {
        throw std::runtime_error("Failed to add framebuffer");
    }

    struct drm_mode_map_dumb map_dumb = {};
    map_dumb.handle = handle;
    if (drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &map_dumb) < 0) {
        throw std::runtime_error("Failed to map dumb buffer");
    }

    map = static_cast<uint8_t*>(mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, map_dumb.offset));
    if (map == MAP_FAILED) {
        throw std::runtime_error("mmap failed");
    }

    std::memset(map, 0, size); // Clear buffer
}

DrmFramebuffer::~DrmFramebuffer() {
    if (map && map != MAP_FAILED) {
        munmap(map, size);
    }

    drmModeRmFB(fd, fb_id);

    struct drm_mode_destroy_dumb destroy = {};
    destroy.handle = handle;
    drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy);
}

uint32_t* DrmFramebuffer::data() {
    return reinterpret_cast<uint32_t*>(map);
}
