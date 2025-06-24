#ifndef DRMCRTC_H
#define DRMCRTC_H

#include <cstdint>
#include <xf86drmMode.h>

class DrmDevice;

class DrmCrtC {
public:
    ~DrmCrtC();

    uint32_t id() const;

    const drmModeCrtc* unwrapped() const;

    // Added method to get the parent DrmDevice
    const DrmDevice& parent() const;

private:
    friend class DrmDevice;
    friend class Framebuffer;

    DrmCrtC(const DrmDevice& device, uint32_t crtc_id);

    const DrmDevice* devparent = nullptr;
    drmModeCrtc* icrtc = nullptr;
};

#endif // DRMCRTC_H
