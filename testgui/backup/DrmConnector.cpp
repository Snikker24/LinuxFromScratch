#include "DrmConnector.h"
#include "DrmDevice.h"
#include "DisplayMode.h"

#include <stdexcept>

DrmConnector::DrmConnector(DrmDevice dparent, uint32_t conn_id)
    : devparent(&dparent)
{
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
