#include "DrmCrtc.h"
#include <xf86drmMode.h>
#include <stdexcept>
#include <iostream>

DrmCrtc::DrmCrtc(const DrmDevice& device, const DrmConnector& connector)
    : fd(device.fd), crtc_id(0), saved_crtc(nullptr)
{
    drmModeRes* res = drmModeGetResources(fd);
    if (!res) throw std::runtime_error("Failed to get DRM resources");

    drmModeEncoder* encoder = drmModeGetEncoder(fd, connector.getRaw().encoder_id);
    if (encoder) {
        crtc_id = encoder->crtc_id;
        drmModeFreeEncoder(encoder);
    }

    if (crtc_id == 0) {
        // Fallback: try finding a CRTC manually
        for (int i = 0; i < res->count_encoders; ++i) {
            drmModeEncoder* enc = drmModeGetEncoder(fd, res->encoders[i]);
            if (!enc) continue;

            for (int j = 0; j < res->count_crtcs; ++j) {
                if (enc->possible_crtcs & (1 << j)) {
                    crtc_id = res->crtcs[j];
                    drmModeFreeEncoder(enc);
                    break;
                }
            }
            if (crtc_id != 0) break;
            drmModeFreeEncoder(enc);
        }
    }

    drmModeFreeResources(res);

    if (crtc_id == 0) throw std::runtime_error("No usable CRTC found");

    saved_crtc = drmModeGetCrtc(fd, crtc_id);
}

DrmCrtc::~DrmCrtc() {
    if (saved_crtc) {
        drmModeSetCrtc(fd, saved_crtc->crtc_id, saved_crtc->buffer_id,
                       saved_crtc->x, saved_crtc->y, &saved_crtc->crtc_id,
                       1, &saved_crtc->mode);
        drmModeFreeCrtc(saved_crtc);
    }
}

void DrmCrtc::setCrtc(const DrmFramebuffer& framebuffer, const DrmConnector& connector, const DisplayMode& mode) {
    uint32_t conn_id = connector.getId();
    drmModeModeInfo* modeInfo = const_cast<drmModeModeInfo*>(&mode.getRaw());
    uint32_t fb_id = framebuffer.getId();
    if (drmModeSetCrtc(fd, crtc_id, fb_id, 0, 0, &conn_id, 1,modeInfo)) {
        throw std::runtime_error("Failed to set CRTC");
    }

}
