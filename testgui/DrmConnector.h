#pragma once
#include <vector>
#include <xf86drmMode.h>
#include "DisplayMode.h"

class DrmConnector {

private:
    drmModeConnector conn;

public:

    DrmConnector(drmModeConnector* connector);

    bool isConnected() const;
    std::vector<DisplayMode> getModes() const;
    uint32_t getId() const;
    const drmModeConnector& getRaw() const;

};
