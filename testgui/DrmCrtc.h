#pragma once

#include <cstdint>
#include "DrmDevice.h"
#include "DrmConnector.h"
#include "DisplayMode.h"
#include "DrmFramebuffer.h"

class DrmCrtc {
public:
    DrmCrtc(DrmDevice& device, uint32_t crtc_id);
    ~DrmCrtc();

    void setCrtc(const DrmConnector& connector, const DisplayMode& mode, const DrmFramebuffer& fb);

private:
    int fd;
    uint32_t crtc_id;
    drmModeCrtc* saved_crtc;
};
