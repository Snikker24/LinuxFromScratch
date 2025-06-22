#include <iostream>
#include <vector>
#include <memory>
#include "DrmDevice.h"
#include "DrmConnector.h"
#include "DisplayMode.h"
#include "DrmFramebuffer.h"
#include "DrmCrtc.h"

int main() {
    try {
        // 1. Discover all valid DRM devices
        auto devices = DrmDevice::fetchAll();
        if (devices.empty()) {
            std::cerr << "No usable DRM devices found.\n";
            return 1;
        }

        DrmDevice& drm = *devices[0];
        std::cout << "Using DRM device: " << drm.path << "\n";

        // 2. Get connectors
        auto connectors = drm.getConnectors();
        if (connectors.empty()) {
            std::cerr << "No connected connectors found\n";
            return 1;
        }
        std::cout << connectors.size() << " connected connector(s) found\n";

        // 3. Pick first connector and get modes
        DrmConnector& conn = connectors[0];
        auto modes = conn.getModes();
        if (modes.empty()) {
            std::cerr << "No modes on connector\n";
            return 1;
        }
        std::cout << "First connector has " << modes.size() << " modes\n";

        DisplayMode mode = modes[0];
        std::cout << "Using mode: " << mode.getWidth() << "x" << mode.getHeight()
                  << " @ " << mode.getFreq() << " Hz\n";

        // 4. Create framebuffer
        DrmFramebuffer fb(drm.fd, mode.getWidth(), mode.getHeight());
        std::cout << "Framebuffer created, id: " << fb.fb_id << "\n";

        // 5. Get DRM resources to find CRTC
        drmModeRes* resources = drmModeGetResources(drm.fd);
        if (!resources) {
            std::cerr << "Failed to get DRM resources\n";
            return 1;
        }

        uint32_t crtc_id = 0;
        for (int i = 0; i < resources->count_crtcs; ++i) {
            crtc_id = resources->crtcs[i];
            if (crtc_id != 0) break;
        }
        drmModeFreeResources(resources);

        if (crtc_id == 0) {
            std::cerr << "No valid CRTC found\n";
            return 1;
        }
        std::cout << "Using CRTC id: " << crtc_id << "\n";

        DrmCrtc crtc(drm.fd, crtc_id);

        // 6. Set CRTC to display the framebuffer
        uint32_t connectors_arr[1] = { conn.getId() };
        crtc.setCrtc(fb.fb_id, connectors_arr, 1, mode.mode);
        std::cout << "CRTC set successfully\n";

        // 7. Fill framebuffer with test pattern
        uint32_t* pixels = fb.data();
        for (uint32_t y = 0; y < mode.getHeight(); ++y) {
            for (uint32_t x = 0; x < mode.getWidth(); ++x) {
                uint32_t color = 0;
                if (x < mode.getWidth() / 3)
                    color = 0xFF0000FF; // Red
                else if (x < 2 * mode.getWidth() / 3)
                    color = 0xFF00FF00; // Green
                else
                    color = 0xFFFF0000; // Blue
                pixels[y * mode.getWidth() + x] = color;
            }
        }
        std::cout << "Framebuffer filled with test pattern\n";

        // Pause before exit
        std::cout << "Press Enter to exit and restore mode...\n";
        std::cin.get();

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
