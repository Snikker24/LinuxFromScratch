#pragma once
#include <string>
#include <vector>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include "DrmConnector.h"

class DrmDevice {
public:
    int fd;
    std::string path;

    DrmDevice(const std::string& device_path);
    ~DrmDevice();

    std::vector<DrmConnector> getConnectors() const;
};
