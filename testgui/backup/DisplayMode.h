#ifndef DISPLAYMODE_H
#define DISPLAYMODE_H

#include <xf86drmMode.h>
#include <cstdint>

#include "DrmConnector.h"
#include "Framebuffer.h"


class DrmConnector;
class Framebuffer;  // Forward declare for friend

class DisplayMode
{
private:
    const drmModeModeInfo imode;
    DrmConnector conn;

    // Private constructor only accessible by friends
    DisplayMode(const DrmConnector& connector, const drmModeModeInfo& mode);

    // Declare friends that can access private members and constructor
    friend class DrmConnector;
    friend class Framebuffer;

public:

    uint16_t width() const;
    uint16_t height() const;
    float frequency() const;

    // Return reference to connector - const to avoid copy and mutation
    const DrmConnector& connector() const;

    // Return const reference to drmModeModeInfo to avoid copies
    const drmModeModeInfo& unwrapped() const;
};

#endif // DISPLAYMODE_H
