#include "drm_dri.h"

using namespace std;

int index(int a, int b, std::string msg){
    if(a == b)
        return a;
    if(a > b)
        swap(a,b);

    int idx;
    do {
        std::cout << msg;
        std::cin >> idx;
    } while(idx < a || idx >= b);

    return idx;
}

int main()
{
    std::vector<DrmDevice> devices = DrmDevice::fetchAll();
    std::cout << "Choose DRM device:\n";
    for(int i = 0; i < devices.size(); i++) {
        std::cout << "[" << i << "]: " << devices[i].path() << "\n";
    }

    int idx = index(0, devices.size(), "Device index: ");
    DrmDevice device = devices[idx];

    std::vector<DrmConnector> connectors = device.connectors();
    std::cout << "Choose DRM device connector:\n";
    for(int i = 0; i < connectors.size(); i++) {
        std::cout << "[" << i << "]: connector-id=" << connectors[i].id() << "\n";
    }

    idx = index(0, connectors.size(), "Connector index: ");
    DrmConnector connector = std::move(connectors[idx]);

    std::vector<DisplayMode> modes = connector.modes();
    std::cout << "Choose DRM connector mode:\n";
    for(int i = 0; i < modes.size(); i++) {
        std::cout << "[" << i << "]: " << modes[i].width() << "x" << modes[i].height() << "@" << modes[i].frequency() << "\n";
    }

    idx = index(0, modes.size(), "Mode index: ");
    DisplayMode mode = std::move(modes[idx]);

    std::vector<DrmCrtC> crtcs = device.controllers();
    std::cout << "Choose DRM CRTC (controller):\n";
    for (int i = 0; i < crtcs.size(); ++i) {
        std::cout << "[" << i << "]: crtc-id=" << crtcs[i].id() << "\n";
    }

    idx = index(0, crtcs.size(), "CRTC index: ");
    DrmCrtC crtc = std::move(crtcs[idx]);

    Framebuffer fb(crtc, mode);
    fb.render();

    uint8_t* pixels = fb.pixels();
    int width = mode.width();
    int height = mode.height();
    int pitch = fb.bytesize() / height; // assumes evenly divisible

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int offset = y * pitch + x * 4;
            pixels[offset + 0] = x * 255 / width;  // Blue
            pixels[offset + 1] = y * 255 / height; // Green
            pixels[offset + 2] = 128;              // Red
            pixels[offset + 3] = 0;                // Alpha (unused)
        }
    }

    return 0;
}
