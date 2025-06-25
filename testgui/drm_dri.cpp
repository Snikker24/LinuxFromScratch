#include "drm_dri.h"


///DRMDEVICE

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

std::vector<DrmConnector> DrmDevice::connectors()
{
    std::vector<DrmConnector> connectors;
    std::cout << "Opened DRM device: " << device_path_ << ", fd = " << fd << std::endl;

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
            DrmCrtC controller(*this, res->crtcs[i]);
            crtcs.push_back(std::move(controller));
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

///DRMCONNECTOR

DrmConnector::DrmConnector(DrmDevice& dparent, uint32_t conn_id)
{
    devparent=&dparent;
    iconnector = drmModeGetConnector(devparent->descriptor(), conn_id);
    if (!iconnector) {
        throw std::runtime_error("Failed to get DRM connector with id: " + std::to_string(conn_id));
    }
}

DrmConnector::~DrmConnector()
{
    if (iconnector) {
        drmModeFreeConnector(iconnector);
        iconnector = nullptr;
    }
}

DrmConnector::DrmConnector(DrmConnector&& other) noexcept {
    devparent = other.devparent;
    iconnector = other.iconnector;

    other.devparent = nullptr;
    other.iconnector = nullptr;
}

DrmConnector& DrmConnector::operator=(DrmConnector&& other) noexcept {
    if (this != &other) {
        // Free existing resource
        if (iconnector) {
            drmModeFreeConnector(iconnector);
        }

        devparent = other.devparent;
        iconnector = other.iconnector;

        other.devparent = nullptr;
        other.iconnector = nullptr;
    }
    return *this;
}

uint32_t DrmConnector::id() const
{
    if (!iconnector)
        throw std::runtime_error("Invalid connector: null pointer");

    return iconnector->connector_id;
}

bool DrmConnector::connected() const
{
    return iconnector && iconnector->connection == DRM_MODE_CONNECTED && iconnector->count_modes > 0;
}

const DrmDevice& DrmConnector::parent() const
{
    if (!devparent)
        throw std::runtime_error("Invalid parent device pointer");

    return *devparent;
}

std::vector<DisplayMode> DrmConnector::modes() const
{
    std::vector<DisplayMode> modes;

    if (!iconnector)
        return modes;

    for (int i = 0; i < iconnector->count_modes; ++i) {
        modes.push_back(DisplayMode(*this, iconnector->modes[i]));
    }

    return modes;
}

const drmModeConnector* DrmConnector::unwrapped() const
{
    return iconnector;
}

///DISPLAYMODE

// Private constructor definition
DisplayMode::DisplayMode(const DrmConnector& connector, const  drmModeModeInfo& mode)
    : imode(mode), conn(&connector)
{

}

DisplayMode::DisplayMode(DisplayMode&& other) noexcept
    : imode(other.imode), conn(other.conn)
{
    other.conn = nullptr;
}

DisplayMode& DisplayMode::operator=(DisplayMode&& other) noexcept
{
    if (this != &other) {
        imode = other.imode;
        conn = other.conn;
        other.conn = nullptr;
    }
    return *this;
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
    if (!conn) throw std::runtime_error("Null connector reference");
    return *conn;
}

const drmModeModeInfo& DisplayMode::unwrapped() const
{
    return imode;
}

///DRMCRTC

DrmCrtC::DrmCrtC(const DrmDevice& device, uint32_t crtc_id)
    : devparent(&device), icrtc(drmModeGetCrtc(device.descriptor(), crtc_id))
{

    idx=-1;
    drmModeRes * res=drmModeGetResources(devparent->descriptor());
    for(uint32_t i=0;i<res->count_crtcs; i++)
        if(res->crtcs[i]==icrtc->crtc_id){
            idx=i;
            break;
        }
    drmModeFreeResources(res);

    if (!icrtc) {
        throw std::runtime_error("Failed to get CRTC");
    }
}

DrmCrtC::~DrmCrtC()
{
    if (icrtc) {
        drmModeFreeCrtc(icrtc);
        icrtc = nullptr;
    }
}

DrmCrtC::DrmCrtC(DrmCrtC&& other) noexcept
    : devparent(other.devparent), icrtc(other.icrtc) {
    other.devparent = nullptr;
    other.icrtc = nullptr;
}

DrmCrtC& DrmCrtC::operator=(DrmCrtC&& other) noexcept {
    if (this != &other) {
        // cleanup current resources if needed
        // (if DrmCrtC manages ownership of icrtc, free it here)
        devparent = other.devparent;
        icrtc = other.icrtc;

        other.devparent = nullptr;
        other.icrtc = nullptr;
        }
    return *this;
}

uint32_t DrmCrtC::id() const
{
    return icrtc ? icrtc->crtc_id : 0;
}

const drmModeCrtc* DrmCrtC::unwrapped() const
{
    return icrtc;
}

const DrmDevice& DrmCrtC::parent() const
{
    if (!devparent) {
        throw std::runtime_error("DrmCrtC has no valid parent device");
    }
    return *devparent;
}

///FRAMEBUFFER

Framebuffer::Framebuffer(DrmCrtC& crtc, DisplayMode& mode): dcrtc(crtc), dmode(mode)
{

    int fd = dcrtc.parent().descriptor();

    drm_mode_create_dumb creq = {};
    creq.width = dmode.width();
    creq.height = dmode.height();
    creq.bpp = 32;

    if (drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq) < 0)
        throw std::runtime_error("Failed to create dumb buffer");

    fbsize = creq.size;
    handle = creq.handle;
    bpr = creq.pitch;

    if (drmModeAddFB(fd, dmode.width(), dmode.height(), 24, 32, bpr, handle, &fb_id) != 0)
        throw std::runtime_error("Failed to add framebuffer");

    drm_mode_map_dumb mreq = {};
    mreq.handle = handle;
    if (drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq) < 0)
        throw std::runtime_error("Failed to map dumb buffer");

    pxMap = static_cast<uint8_t*>(
        mmap(nullptr, fbsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, mreq.offset)
    );
    if (pxMap == MAP_FAILED)
        throw std::runtime_error("Failed to mmap framebuffer");
}

Framebuffer::~Framebuffer()
{
    if (!pxMap)
        return;

    int fd = dcrtc.parent().descriptor();

    munmap(pxMap, fbsize);
    drmModeRmFB(fd, fb_id);

    drm_mode_destroy_dumb dreq = {};
    dreq.handle = handle;
    drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
}

uint32_t Framebuffer::id() const
{
    return fb_id;
}

uint32_t Framebuffer::bytesize() const
{
    return fbsize;
}

Framebuffer::PxMap Framebuffer::pixels() const
{
    return pxMap;
}

const DisplayMode& Framebuffer::mode() const
{
    return dmode;
}

const DrmCrtC& Framebuffer::controller() const
{
    return dcrtc;
}

void Framebuffer::render()
{


    drmModeEncoder * enc= drmModeGetEncoder(dcrtc.parent().descriptor(), dmode.connector().unwrapped()->encoder_id);

    if(!(enc->possible_crtcs && (1<< dcrtc.index()))){
        throw std::runtime_error("Selected CRTC is not compatible with encoder");
    }

    uint32_t id[]{dmode.connector().id()};
    drmModeModeInfo * mode_ptr=dmode.connector().unwrapped()->modes;
    int ret = drmModeSetCrtc(
        dcrtc.parent().descriptor(),        // DRM file descriptor
        dcrtc.id(),          // CRTC ID
        fb_id,             // Framebuffer ID
        0, 0,               // Position X, Y on screen
        id,    // Connector ID array
        1,               // Connector count
        mode_ptr          // Display mode info
    );

    if (ret) {
        perror("drmModeSetCrtc failed");
    } else {
        std::cout << "Framebuffer successfully set to CRTC.\n";
    }

    if (pxMap && fbsize > 0) {
        memset(pxMap, 0, fbsize);  // Clear framebuffer to black
    }
}

Framebuffer::Framebuffer(Framebuffer&& other) noexcept
    : fb_id(other.fb_id), handle(other.handle), bpr(other.bpr), fbsize(other.fbsize),
      pxMap(other.pxMap), dmode(other.dmode), dcrtc(other.dcrtc)
{
    other.fb_id = 0;
    other.handle = 0;
    other.bpr = 0;
    other.fbsize = 0;
    other.pxMap = nullptr;
}

Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept
{
    if (this != &other) {
        // Clean up existing framebuffer resources
        if (pxMap) {
            int fd = dcrtc.parent().descriptor();
            munmap(pxMap, fbsize);
            drmModeRmFB(fd, fb_id);
            drm_mode_destroy_dumb dreq = {};
            dreq.handle = handle;
            drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
        }

        fb_id = other.fb_id;
        handle = other.handle;
        bpr = other.bpr;
        fbsize = other.fbsize;
        pxMap = other.pxMap;

        // **Do NOT assign dmode and dcrtc references** — they must stay bound to the same objects.

        other.fb_id = 0;
        other.handle = 0;
        other.bpr = 0;
        other.fbsize = 0;
        other.pxMap = nullptr;
    }
    return *this;
}

