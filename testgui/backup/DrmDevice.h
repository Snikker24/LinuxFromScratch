#ifndef DRMDEVICE_H
#define DRMDEVICE_H


#include <xf86drm.h>
#include <xf86drmMode.h>
#include <string>
#include <vector>
#include <unistd.h>

#include "DisplayMode.h"
#include "DrmCrtC.h"



class DrmConnector;
class DrmCrtC;

class DrmDevice {
private:
    int fd = -1;
    std::string device_path_;

public:
    explicit DrmDevice(const std::string& device_path);
    ~DrmDevice();

    int descriptor() const;
    std::string path() const;

    std::vector<DrmConnector> connectors() const;
    std::vector<DrmCrtC> controllers() const;

    static std::vector<DrmDevice> fetchAll();

    // Non-copyable (file descriptors must be unique)
    //DrmDevice(const DrmDevice&) = delete;
    //DrmDevice& operator=(const DrmDevice&) = delete;

    // Movable
    /*DrmDevice(DrmDevice&& other) noexcept {
        fd = other.fd;
        device_path_ = std::move(other.device_path_);
        other.fd = -1;
    }

    DrmDevice& operator=(DrmDevice&& other) noexcept {
        if (this != &other) {
            if (fd >= 0)
                close(fd);
            fd = other.fd;
            device_path_ = std::move(other.device_path_);
            other.fd = -1;
        }
        return *this;
    }*/
};

#endif // DRMDEVICE_H
