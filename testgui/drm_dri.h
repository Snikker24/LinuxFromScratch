#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <memory>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <filesystem>
#include <drm.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

//device wrapper class


class DrmDevice{

private:
    int fd;
    drmModeRes * res;
    std::string path;
    DrmDevice(std::string device_path);

public:

    ~DrmDevice();
    static std::vector<DrmDevice> fetchAll();

    int descriptor() const;

    std::string devicePath() const;

    //Connectors wrapper class
    class DrmConnector{

    private:
        drmModeConnector * iconnector;
        DrmDevice * devparent;
        DrmConnector(DrmDevice parent, uint32_t conn_id);
        friend class DrmDevice;

    public:

        ~DrmConnector();

        int id() const;

        bool connected() const;

        DrmDevice parent();

        const drmModeConnector* unwrapped();

        class DisplayMode{

        private:
            const drmModeModeInfo imode;
            DrmDevice::DrmConnector* conn;
            DisplayMode(DrmConnector connector, drmModeModeInfo mode);
            friend class DrmConnector;
            friend class Framebuffer;
        public:


            uint16_t width() const;

            uint16_t height() const;

            float frequency() const;

            DrmConnector connector();

            const drmModeModeInfo unwrapped();

        };

        std::vector<DisplayMode> modes() const;

    };

    std::vector<DrmDevice::DrmConnector> connectors() const;

    class DrmCrtc{

    private:
        const drmModeCrtc icrtc;
        DrmDevice * devparent;
        DrmCrtc(DrmDevice parent, drmModeCrtc crtc);

    public:

        DrmDevice parent() const;

        void setCrtc();

        const drmModeCrtc unwrapped();


    };

};


class Framebuffer{

    private:
        uint32_t fb_id;
        uint32_t handle;
        uint32_t bpr;
        uint32_t fbsize;
        uint8_t* pxMap = nullptr;
        DrmDevice::DrmConnector::DisplayMode mode;
    public:
        typedef uint32_t PxMap;
        Framebuffer(DrmDevice::DrmConnector::DisplayMode mode);
        ~Framebuffer();

        uint32_t id() const;
        uint32_t bytesize() const;
        PxMap pixels();


    };
