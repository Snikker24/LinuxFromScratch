#include "Framebuffer.h"
#include <stdexcept>
#include <cstring>      // for memset
#include <sys/mman.h>   // for mmap, munmap
#include <xf86drm.h>    // for DRM_IOCTL_MODE_CREATE_DUMB etc
#include <xf86drmMode.h>

#include "DisplayMode.h"
#include "DrmCrtC.h"
#include "DrmDevice.h"

Framebuffer::Framebuffer(DrmCrtC crtc, DisplayMode mode)
{

    dcrtc=crtc;
    dmode=mode;
    int fd = dcrtc.parent().descriptor();

    drm_mode_create_dumb creq = {};
    creq.width = dmode.width();
    creq.height = dmode.height();
    creq.bpp = 32;

    if (drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq) < 0)
        throw std::runtime_error("Failed to create dumb buffer");

    fbsize = creq.size;
    handle = creq.handle;
    bpr = creq.pitch;

    if (drmModeAddFB(fd, dmode.width(), dmode.height(), 24, 32, bpr, handle, &fb_id) != 0)
        throw std::runtime_error("Failed to add framebuffer");

    drm_mode_map_dumb mreq = {};
    mreq.handle = handle;
    if (drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq) < 0)
        throw std::runtime_error("Failed to map dumb buffer");

    pxMap = static_cast<uint8_t*>(
        mmap(nullptr, fbsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, mreq.offset)
    );
    if (pxMap == MAP_FAILED)
        throw std::runtime_error("Failed to mmap framebuffer");
}

Framebuffer::~Framebuffer()
{
    if (!pxMap)
        return;

    int fd = dcrtc.parent().descriptor();

    munmap(pxMap, fbsize);
    drmModeRmFB(fd, fb_id);

    drm_mode_destroy_dumb dreq = {};
    dreq.handle = handle;
    drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
}

uint32_t Framebuffer::id() const
{
    return fb_id;
}

uint32_t Framebuffer::bytesize() const
{
    return fbsize;
}

Framebuffer::PxMap Framebuffer::pixels() const
{
    return pxMap;
}

const DisplayMode& Framebuffer::mode() const
{
    return dmode;
}

const DrmCrtC& Framebuffer::controller() const
{
    return dcrtc;
}

void Framebuffer::render()
{
    if (pxMap && fbsize > 0) {
        std::memset(pxMap, 0, fbsize);  // Clear framebuffer to black
    }
}

Framebuffer::Framebuffer(Framebuffer&& other) noexcept
    : fb_id(other.fb_id), handle(other.handle), bpr(other.bpr), fbsize(other.fbsize),
      pxMap(other.pxMap), dmode(std::move(other.dmode)), dcrtc(std::move(other.dcrtc))
{
    other.fb_id = 0;
    other.handle = 0;
    other.bpr = 0;
    other.fbsize = 0;
    other.pxMap = nullptr;
}
/*
Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept
{
    if (this != &other) {
        this->~Framebuffer(); // Cleanup current resources

        fb_id = other.fb_id;
        handle = other.handle;
        bpr = other.bpr;
        fbsize = other.fbsize;
        pxMap = other.pxMap;
        dmode = std::move(other.dmode);
        dcrtc = std::move(other.dcrtc);

        other.fb_id = 0;
        other.handle = 0;
        other.bpr = 0;
        other.fbsize = 0;
        other.pxMap = nullptr;
    }
    return *this;
}
*/
