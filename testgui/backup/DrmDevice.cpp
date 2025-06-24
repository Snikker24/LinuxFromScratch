#include "DrmDevice.h"
#include "DrmConnector.h"
#include "DrmCrtC.h" // Needed for returning DrmCrtC objects

#include <fcntl.h>          // open
#include <unistd.h>         // close
#include <stdexcept>
#include <filesystem>
#include <xf86drm.h>
#include <xf86drmMode.h>

DrmDevice::DrmDevice(const std::string& device_path)
    : device_path_(device_path)
{
    fd = open(device_path_.c_str(), O_RDWR | O_CLOEXEC);
    if (fd < 0) {
        throw std::runtime_error("Failed to open DRM device: " + device_path_);
    }
}

DrmDevice::~DrmDevice()
{
    if (fd >= 0) {
        close(fd);
    }
}

int DrmDevice::descriptor() const
{
    return fd;
}

std::string DrmDevice::path() const
{
    return device_path_;
}

std::vector<DrmConnector> DrmDevice::connectors() const
{
    std::vector<DrmConnector> connectors;

    drmModeRes* res = drmModeGetResources(fd);
    if (!res) {
        throw std::runtime_error("Failed to get DRM resources for device: " + device_path_);
    }

    for (int i = 0; i < res->count_connectors; ++i) {
        try {
            DrmConnector conn(*this, res->connectors[i]);
            if (!conn.modes().empty()) {
                connectors.push_back(std::move(conn));
            }
        } catch (...) {
            // Skip connectors that failed to initialize
        }
    }

    drmModeFreeResources(res);
    return connectors;
}

std::vector<DrmCrtC> DrmDevice::controllers() const
{
    std::vector<DrmCrtC> crtcs;

    drmModeRes* res = drmModeGetResources(fd);
    if (!res) {
        throw std::runtime_error("Failed to get DRM resources for CRTCs");
    }

    for (int i = 0; i < res->count_crtcs; ++i) {
        try {
            crtcs.emplace_back(*this, res->crtcs[i]);
        } catch (...) {
            // Ignore failures for individual CRTCs
        }
    }

    drmModeFreeResources(res);
    return crtcs;
}

std::vector<DrmDevice> DrmDevice::fetchAll()
{
    std::vector<DrmDevice> devices;

    for (const auto& entry : std::filesystem::directory_iterator("/dev/dri")) {
        if (entry.path().string().find("card") != std::string::npos) {
            try {
                DrmDevice dev(entry.path().string());
                if (!dev.connectors().empty()) {
                    devices.push_back(std::move(dev));
                }
            } catch (...) {
                // Ignore devices that failed to open or have no connectors
            }
        }
    }

    return devices;
}
