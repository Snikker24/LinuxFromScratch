#include "drm_dri.h"

//DrmDevice
DrmDevice::DrmDevice(std::string device_path){

    path=device_path;
    fd = open(path.c_str(), O_RDWR | O_CLOEXEC);
    if (fd < 0) throw std::runtime_error("Failed to open DRM device: " + path);
}


DrmDevice::~DrmDevice(){

    if (fd >= 0) close(fd);

}

std::vector<DrmDevice::DrmConnector> DrmDevice::connectors() const{

    std::vector<DrmConnector> connectors;

    drmModeRes* res = drmModeGetResources(this->fd);
    if (!res) {
        std::cerr << "drmModeGetResources failed\n";
        return connectors;
    }

    for (int i = 0; i < res->count_connectors; ++i) {
        drmModeConnector* conn = drmModeGetConnector(fd, res->connectors[i]);
        if (conn && conn->connection == DRM_MODE_CONNECTED && conn->count_modes > 0) {
            connectors.emplace_back(DrmConnector(*this, *conn));
        } else if (conn) {
            drmModeFreeConnector(conn);
        }
    }

    drmModeFreeResources(res);
    return connectors;

}

std::vector<DrmDevice> DrmDevice::fetchAll(){

    std::vector<DrmDevice> devices;

    for (const auto& entry : std::filesystem::directory_iterator("/dev/dri")) {
        if (entry.path().string().find("card") != std::string::npos) {
            try {
                std::cout<<entry.path()<<"\n";
                DrmDevice dev = DrmDevice(entry.path());
                std::vector<DrmDevice::DrmConnector> conns = dev.connectors();
                if (!conns.empty()) {
                    devices.push_back(dev);
                }
            } catch (...) {
                // ignore devices we cannot use
            }
        }
    }

    return devices;

}

int DrmDevice::descriptor() const{
    return fd;
}

std::string DrmDevice::devicePath() const{

    return path;

}

//DrmConnector
DrmDevice::DrmConnector::DrmConnector(DrmDevice parent, drmModeConnector connector):iconnector(connector){

    this->devparent=&parent;


}

int DrmDevice::DrmConnector::id() const{

    return this->iconnector.connector_id;

}

bool DrmDevice::DrmConnector::connected() const{

    return this->iconnector.connection==DRM_MODE_CONNECTED && iconnector.count_modes > 0;

}

const drmModeConnector DrmDevice::DrmConnector::unwrapped(){

    return this->iconnector;

}

DrmDevice DrmDevice::DrmConnector::parent(){

    return *devparent;

}

//DisplayMode
DrmDevice::DrmConnector::DisplayMode::DisplayMode(DrmDevice::DrmConnector connector, drmModeModeInfo mode):imode(mode){

    conn=&connector;

}

uint16_t DrmDevice::DrmConnector::DisplayMode::width() const {
    return imode.hdisplay;
}

uint16_t DrmDevice::DrmConnector::DisplayMode::height() const {
    return imode.vdisplay;
}

float DrmDevice::DrmConnector::DisplayMode::frequency() const {
    // frequency in Hz
    return imode.clock * 1000.0f / (imode.htotal * imode.vtotal);
}

DrmDevice::DrmConnector DrmDevice::DrmConnector::DisplayMode::connector(){

    return * conn;

}


const drmModeModeInfo DrmDevice::DrmConnector::DisplayMode::unwrapped(){

    return this->imode;

}

//DisplayMode-END

std::vector<DrmDevice::DrmConnector::DisplayMode> DrmDevice::DrmConnector::modes() const{

    std::vector<DrmDevice::DrmConnector::DisplayMode> modes;
    for (int i = 0; i < iconnector.count_modes; ++i) {
        modes.push_back(DisplayMode(*this,iconnector.modes[i]));
    }
    return modes;

}

//DrmConnector-END

//DrmCrtc
DrmDevice::DrmCrtc::DrmCrtc(DrmDevice parent, drmModeCrtc crtc):icrtc(crtc){

    this->devparent=&parent;

}

DrmDevice DrmDevice::DrmCrtc::parent() const{

    return *devparent;

}


const drmModeCrtc DrmDevice::DrmCrtc::unwrapped(){

    return icrtc;

}
