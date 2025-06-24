#ifndef DRM_DRI_H
#define DRM_DRI_H
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdint>
#include <memory>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <filesystem>
#include <drm.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

class DrmDevice;
class DrmConnector;
class DrmCrtC;
class DisplayMode;
class Framebuffer;

class DrmDevice {
private:
    int fd = -1;
    std::string device_path_;

public:
    explicit DrmDevice(const std::string& device_path);
    ~DrmDevice();

    int descriptor() const;
    std::string path() const;

    std::vector<DrmConnector> connectors();
    std::vector<DrmCrtC> controllers() const;

    static std::vector<DrmDevice> fetchAll();

};

class DrmConnector
{
public:
    virtual ~DrmConnector();

    uint32_t id() const;
    bool connected() const;

    // Return DrmDevice by reference (const), no copy
    const DrmDevice& parent() const;

    std::vector<DisplayMode> modes() const;

    const drmModeConnector* unwrapped() const;

    DrmConnector(const DrmConnector&) = delete;
    DrmConnector& operator=(const DrmConnector&) = delete;

    DrmConnector(DrmConnector&& other) noexcept;
    DrmConnector& operator=(DrmConnector&& other) noexcept;

private:
    friend class DrmDevice;
    DrmConnector(DrmDevice& devparent, uint32_t conn_id);
    // Pass DrmDevice by reference to avoid copying and to keep pointer valid

    drmModeConnector* iconnector{nullptr};
    DrmDevice* devparent{nullptr};
};

class DisplayMode
{
private:
    drmModeModeInfo imode;
    const DrmConnector* conn;


    // Private constructor only accessible by friends
    DisplayMode(const DrmConnector& connector, const drmModeModeInfo& mode);

    // Declare friends that can access private members and constructor
    friend class DrmConnector;
    friend class Framebuffer;

public:

    uint16_t width() const;
    uint16_t height() const;
    float frequency() const;

    // Move constructor and move assignment
    DisplayMode(DisplayMode&& other) noexcept;
    DisplayMode& operator=(DisplayMode&& other) noexcept;

    // Delete copy operations
    DisplayMode(const DisplayMode&) = delete;
    DisplayMode& operator=(const DisplayMode&) = delete;

    // Return reference to connector - const to avoid copy and mutation
    const DrmConnector& connector() const;

    // Return const reference to drmModeModeInfo to avoid copies
    const drmModeModeInfo& unwrapped() const;
};

class DrmCrtC {
public:
    ~DrmCrtC();

    uint32_t id() const;

    const drmModeCrtc* unwrapped() const;

    // Added method to get the parent DrmDevice
    const DrmDevice& parent() const;

    DrmCrtC(const DrmCrtC&) = delete;
    DrmCrtC& operator=(const DrmCrtC&) = delete;

    DrmCrtC(DrmCrtC&& other) noexcept;
    DrmCrtC& operator=(DrmCrtC&& other) noexcept;

private:
    friend class DrmDevice;
    friend class Framebuffer;

    DrmCrtC(const DrmDevice& device, uint32_t crtc_id);

    const DrmDevice* devparent = nullptr;
    drmModeCrtc* icrtc = nullptr;
};

class Framebuffer {
private:
    uint32_t fb_id = 0;
    uint32_t handle = 0;
    uint32_t bpr = 0;      // bytes per row (pitch)
    uint32_t fbsize = 0;   // framebuffer size in bytes
    uint8_t* pxMap = nullptr;
    DisplayMode& dmode;
    DrmCrtC& dcrtc;

public:
    using PxMap = uint8_t*;

    Framebuffer(DrmCrtC& crtc, DisplayMode& mode);
    ~Framebuffer();

    uint32_t id() const;
    uint32_t bytesize() const;
    PxMap pixels() const;
    const DisplayMode& mode() const;
    const DrmCrtC& controller() const;

    void render();

    // Non-copyable to avoid multiple owners of pxMap & fb resources
    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;

    // Movable
    Framebuffer(Framebuffer&& other) noexcept;
    Framebuffer& operator=(Framebuffer&& other) noexcept;
};

#endif // DRM_DRI_H
