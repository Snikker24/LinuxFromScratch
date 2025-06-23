#ifndef DRM_CRTC_H
#define DRM_CRTC_H

#include "DrmDevice.h"
#include "DrmConnector.h"
#include "DrmFramebuffer.h"
#include "DisplayMode.h"
#include <stdint.h>

class DrmCrtc {
public:
    DrmCrtc(const DrmDevice& device, const DrmConnector& connector);
    ~DrmCrtc();

    void setCrtc(const DrmFramebuffer& framebuffer, const DrmConnector& connector, const DisplayMode& mode);

private:
    int fd;
    uint32_t crtc_id;
    drmModeCrtc* saved_crtc;
};

#endif
