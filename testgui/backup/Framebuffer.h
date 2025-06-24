#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <cstdint>
#include <cstring>
#include <sys/mman.h>
#include <stdexcept>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm/drm_mode.h>

#include "DisplayMode.h"

class DrmDevice;
class DrmCrtC;
class DisplayMode;

class Framebuffer {
private:
    uint32_t fb_id = 0;
    uint32_t handle = 0;
    uint32_t bpr = 0;      // bytes per row (pitch)
    uint32_t fbsize = 0;   // framebuffer size in bytes
    uint8_t* pxMap = nullptr;
    DisplayMode dmode;
    DrmCrtC dcrtc;

public:
    using PxMap = uint8_t*;

    explicit Framebuffer(DrmCrtC crtc, DisplayMode mode);
    ~Framebuffer();

    uint32_t id() const;
    uint32_t bytesize() const;
    PxMap pixels() const;
    const DisplayMode& mode() const;
    const DrmCrtC& controller() const;

    void render();

    // Non-copyable to avoid multiple owners of pxMap & fb resources
    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;

    // Movable
    Framebuffer(Framebuffer&& other) noexcept;
    //Framebuffer& operator=(Framebuffer&& other) noexcept;
};

#endif // FRAMEBUFFER_H
