#pragma once
#include <string>
#include <memory>
#include <vector>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include "DrmConnector.h"

class DrmDevice {

private:
    int fd;
    std::string path;

public:

    int getFd() const;

    std::string getPath() const;

    DrmDevice(const std::string& device_path);
    ~DrmDevice();

    std::vector<DrmConnector> getConnectors() const;

    static std::vector<std::unique_ptr<DrmDevice>> fetchAll();

};
