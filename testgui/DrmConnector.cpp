#include "DrmConnector.h"
#include <stdexcept>

DrmConnector::DrmConnector(drmModeConnector* connector){
    if (!connector) throw std::runtime_error("Null connector passed");
    else conn=*connector;

    drmModeFreeConnector(connector);
}

bool DrmConnector::isConnected() const {
    return conn.connection == DRM_MODE_CONNECTED && conn.count_modes > 0;
}

std::vector<DisplayMode> DrmConnector::getModes() const {
    std::vector<DisplayMode> modes;
    for (int i = 0; i < conn.count_modes; ++i) {
        modes.emplace_back(conn.modes[i]);
    }
    return modes;
}

uint32_t DrmConnector::getId() const {
    return conn.connector_id;
}


const drmModeConnector& DrmConnector::getRaw() const {
    return conn;
}
