#include "DrmCrtC.h"
#include "DrmDevice.h"
#include <stdexcept>

DrmCrtC::DrmCrtC(const DrmDevice& device, uint32_t crtc_id)
    : devparent(&device), icrtc(drmModeGetCrtc(device.descriptor(), crtc_id))
{
    if (!icrtc) {
        throw std::runtime_error("Failed to get CRTC");
    }
}

DrmCrtC::~DrmCrtC()
{
    if (icrtc) {
        drmModeFreeCrtc(icrtc);
        icrtc = nullptr;
    }
}

uint32_t DrmCrtC::id() const
{
    return icrtc ? icrtc->crtc_id : 0;
}

const drmModeCrtc* DrmCrtC::unwrapped() const
{
    return icrtc;
}

const DrmDevice& DrmCrtC::parent() const
{
    if (!devparent) {
        throw std::runtime_error("DrmCrtC has no valid parent device");
    }
    return *devparent;
}
