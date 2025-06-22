#pragma once
#include <xf86drmMode.h>

class DisplayMode {
public:
    drmModeModeInfo mode;

    DisplayMode(const drmModeModeInfo& mode);
    uint16_t getWidth() const;
    uint16_t getHeight() const;
    float getFreq() const;
};
