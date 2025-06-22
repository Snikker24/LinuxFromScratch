#include "DrmCrtc.h"
#include <stdexcept>
#include <xf86drmMode.h>

DrmCrtc::DrmCrtc(int drm_fd, uint32_t crtc_id) : fd(drm_fd) {
    crtc = drmModeGetCrtc(fd, crtc_id);
    if (!crtc) throw std::runtime_error("Failed to get CRTC");
}

DrmCrtc::~DrmCrtc() {
    if (crtc) drmModeFreeCrtc(crtc);
}

void DrmCrtc::setCrtc(uint32_t fb_id, uint32_t* connectors, int count, const drmModeModeInfo& mode) {
    if (drmModeSetCrtc(fd, crtc->crtc_id, fb_id, 0, 0, connectors, count, const_cast<drmModeModeInfo*>(&mode))) {
        throw std::runtime_error("Failed to set CRTC");
    }
}
