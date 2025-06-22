#include "DrmDevice.h"
#include <fcntl.h>
#include <filesystem>
#include <unistd.h>
#include <stdexcept>
#include <dirent.h>
#include <iostream>

DrmDevice::DrmDevice(const std::string& device_path) : path(device_path) {
    fd = open(path.c_str(), O_RDWR | O_CLOEXEC);
    if (fd < 0) throw std::runtime_error("Failed to open DRM device: " + path);
}

DrmDevice::~DrmDevice() {
    if (fd >= 0) close(fd);
}

std::vector<DrmConnector> DrmDevice::getConnectors() const {
    std::vector<DrmConnector> connectors;

    drmModeRes* res = drmModeGetResources(fd);
    if (!res) {
        std::cerr << "drmModeGetResources failed\n";
        return connectors;
    }

    for (int i = 0; i < res->count_connectors; ++i) {
        drmModeConnector* conn = drmModeGetConnector(fd, res->connectors[i]);
        if (conn && conn->connection == DRM_MODE_CONNECTED && conn->count_modes > 0) {
            connectors.emplace_back(conn);
        } else if (conn) {
            drmModeFreeConnector(conn);
        }
    }

    drmModeFreeResources(res);
    return connectors;
}

std::vector<std::unique_ptr<DrmDevice>> DrmDevice::fetchAll() {
    std::vector<std::unique_ptr<DrmDevice>> devices;

    for (const auto& entry : std::filesystem::directory_iterator("/dev/dri")) {
        if (entry.path().string().find("card") != std::string::npos) {
            try {
                auto dev = std::make_unique<DrmDevice>(entry.path());
                auto conns = dev->getConnectors();
                if (!conns.empty()) {
                    devices.push_back(std::move(dev));
                }
            } catch (...) {
                // ignore devices we cannot use
            }
        }
    }

    return devices;
}
