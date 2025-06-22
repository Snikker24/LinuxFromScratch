#include "DrmConnector.h"
#include <stdexcept>

DrmConnector::DrmConnector(drmModeConnector* connector) : conn(connector) {
    if (!conn) throw std::runtime_error("Null connector passed");
}

DrmConnector::~DrmConnector() {
    if (conn) drmModeFreeConnector(conn);
}

bool DrmConnector::isConnected() const {
    return conn->connection == DRM_MODE_CONNECTED && conn->count_modes > 0;
}

std::vector<DisplayMode> DrmConnector::getModes() const {
    std::vector<DisplayMode> modes;
    for (int i = 0; i < conn->count_modes; ++i) {
        modes.emplace_back(conn->modes[i]);
    }
    return modes;
}

uint32_t DrmConnector::getId() const {
    return conn->connector_id;
}
