#include "DisplayMode.h"

DisplayMode::DisplayMode(const drmModeModeInfo& m) : mode(m) {}

uint16_t DisplayMode::getWidth() const {
    return mode.hdisplay;
}

uint16_t DisplayMode::getHeight() const {
    return mode.vdisplay;
}

float DisplayMode::getFreq() const {
    // frequency in Hz
    return mode.clock * 1000.0f / (mode.htotal * mode.vtotal);
}

const drmModeModeInfo& DisplayMode::getRaw() const {

    return mode;

}
