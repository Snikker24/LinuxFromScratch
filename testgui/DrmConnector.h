#pragma once
#include <vector>
#include <xf86drmMode.h>
#include "DisplayMode.h"

class DrmConnector {
public:
    drmModeConnector* conn;

    explicit DrmConnector(drmModeConnector* connector);
    ~DrmConnector();

    bool isConnected() const;
    std::vector<DisplayMode> getModes() const;
    uint32_t getId() const;
};
