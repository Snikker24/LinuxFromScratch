#include "DrmCrtc.h"
#include <stdexcept>
#include <iostream>
#include <xf86drmMode.h>

DrmCrtc::DrmCrtc(DrmDevice& device, uint32_t crtc_id)
    : fd(device.getFd()), crtc_id(crtc_id), saved_crtc(nullptr) {
    saved_crtc = drmModeGetCrtc(fd, crtc_id);
    if (!saved_crtc) {
        throw std::runtime_error("Failed to get current CRTC");
    }
}

DrmCrtc::~DrmCrtc() {
    if (saved_crtc) {
        drmModeSetCrtc(fd, saved_crtc->crtc_id, saved_crtc->buffer_id, saved_crtc->x, saved_crtc->y,
                       &saved_crtc->crtc_id, 1, &saved_crtc->mode);
        drmModeFreeCrtc(saved_crtc);
    }
}

void DrmCrtc::setCrtc(const DrmConnector& connector,
                      const DisplayMode& mode,
                      const DrmFramebuffer& fb) {
    drmModeModeInfoPtr mode_ptr = const_cast<drmModeModeInfo*>(&mode.getRaw()); // Safe here
    uint32_t connector_id = connector.getId();

    if (drmModeSetCrtc(fd,
                       crtc_id,
                       fb.getId(),
                       0,
                       0,
                       &connector_id,
                       1,
                       mode_ptr) != 0) {
        throw std::runtime_error("Failed to set CRTC");
    }
}
