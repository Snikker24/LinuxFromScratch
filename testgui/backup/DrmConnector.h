#ifndef DRMCONNECTOR_H
#define DRMCONNECTOR_H

#include <vector>
#include <cstdint>
#include <xf86drm.h>
#include <xf86drmMode.h>


#include "DrmDevice.h"
#include "DisplayMode.h"

class DrmDevice;
class DisplayMode;

class DrmConnector
{
public:
    // Pass DrmDevice by reference to avoid copying and to keep pointer valid
    DrmConnector(DrmDevice devparent, uint32_t conn_id);
    virtual ~DrmConnector();

    uint32_t id() const;
    bool connected() const;

    // Return DrmDevice by reference (const), no copy
    const DrmDevice& parent() const;

    std::vector<DisplayMode> modes() const;

    const drmModeConnector* unwrapped() const;

private:
    drmModeConnector* iconnector{nullptr};
    DrmDevice* devparent{nullptr};
};

#endif // DRMCONNECTOR_H
