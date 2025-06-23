#include <iostream>
#include <vector>
#include "DrmDevice.h"
#include "DrmConnector.h"
#include "DisplayMode.h"
#include "DrmFramebuffer.h"
#include "DrmCrtc.h"

int main() {
    try {
        // 1. Discover and open first valid DRM device
        auto devices = DrmDevice::fetchAll();
        if (devices.empty()) {
            std::cerr << "No valid DRM devices found\n";
            return 1;
        }
        DrmDevice& drm = *devices[0];
        std::cout << "DRM device opened\n";

        // 2. Get connected connectors
        auto connectors = drm.getConnectors();
        if (connectors.empty()) {
            std::cerr << "No connected connectors found\n";
            return 1;
        }
        DrmConnector& conn = connectors[0];
        std::cout << "Connector found with id " << conn.getId() << "\n";

        // 3. Get modes for connector
        auto modes = conn.getModes();
        if (modes.empty()) {
            std::cerr << "No modes available for connector\n";
            return 1;
        }
        DisplayMode mode = modes[0];
        std::cout << "Selected mode: " << mode.getWidth() << "x" << mode.getHeight()
                  << " @ " << mode.getFreq() << " Hz\n";

        // 4. Create framebuffer for selected mode
        DrmFramebuffer fb(drm, mode);
        std::cout << "Framebuffer created with id: " << fb.getId() << "\n";

        // 5. Create CRTC wrapper for connector and set framebuffer
        DrmCrtc crtc(drm, conn);
        crtc.setCrtc(fb, conn, mode);
        std::cout << "CRTC set successfully\n";

        // 6. Draw simple test pattern
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

        // 7. Wait for user input before exiting
        std::cout << "Press Enter to exit and restore display...\n";
        std::cin.get();

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
