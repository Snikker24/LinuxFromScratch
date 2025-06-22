#pragma once
#include <xf86drmMode.h>

class DrmCrtc {
public:
    int fd;
    drmModeCrtc* crtc;

    DrmCrtc(int drm_fd, uint32_t crtc_id);
    ~DrmCrtc();

    void setCrtc(uint32_t fb_id, uint32_t* connectors, int count, const drmModeModeInfo& mode);
};
