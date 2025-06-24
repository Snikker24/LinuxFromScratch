#include "DisplayMode.h"
#include "DrmConnector.h"

// Private constructor definition
DisplayMode::DisplayMode(const DrmConnector& connector, const drmModeModeInfo& mode)
    : imode(mode), conn(connector)
{

}

uint16_t DisplayMode::width() const
{
    return imode.hdisplay;
}

uint16_t DisplayMode::height() const
{
    return imode.vdisplay;
}

float DisplayMode::frequency() const
{
    // frequency in Hz = pixel clock (kHz) * 1000 / (htotal * vtotal)
    return static_cast<float>(imode.clock * 1000) / (imode.htotal * imode.vtotal);
}

const DrmConnector& DisplayMode::connector() const
{
    return conn;
}

const drmModeModeInfo& DisplayMode::unwrapped() const
{
    return imode;
}
