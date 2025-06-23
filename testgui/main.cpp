#include <iostream>
#include <vector>
#include <string>
#include "DrmDevice.h"
#include "DrmConnector.h"
#include "DisplayMode.h"
#include "DrmFramebuffer.h"
#include "DrmCrtc.h"

int main() {
    try {
        // 1. List available DRM devices
        auto devices = DrmDevice::fetchAll();
        if (devices.empty()) {
            std::cerr << "No DRM devices found.\n";
            return 1;
        }

        std::cout << "Available DRM devices:\n";
        for (size_t i = 0; i < devices.size(); ++i) {
            std::cout << " [" << i << "] " << devices[i]->getPath() << "\n";
        }

        std::cout << "Select device index: ";
        size_t device_index;
        std::cin >> device_index;
        if (device_index >= devices.size()) {
            std::cerr << "Invalid device index\n";
            return 1;
        }

        DrmDevice& drm = *devices[device_index];
        std::cout << "Selected DRM device: " << drm.getPath() << "\n";

        // 2. Select connector
        auto connectors = drm.getConnectors();
        if (connectors.empty()) {
            std::cerr << "No connected connectors found\n";
            return 1;
        }

        std::cout << "Available connectors:\n";
        for (size_t i = 0; i < connectors.size(); ++i) {
            std::cout << " [" << i << "] Connector ID: " << connectors[i].getId()
                      << ", Modes: " << connectors[i].getModes().size() << "\n";
        }

        std::cout << "Select connector index: ";
        size_t conn_index;
        std::cin >> conn_index;
        if (conn_index >= connectors.size()) {
            std::cerr << "Invalid connector index\n";
            return 1;
        }

        DrmConnector& conn = connectors[conn_index];
        auto modes = conn.getModes();
        if (modes.empty()) {
            std::cerr << "No modes available for selected connector\n";
            return 1;
        }

        // 3. Select display mode
        std::cout << "Available modes:\n";
        for (size_t i = 0; i < modes.size(); ++i) {
            const DisplayMode& m = modes[i];
            std::cout << " [" << i << "] " << m.getWidth() << "x" << m.getHeight()
                      << " @ " << m.getFreq() << " Hz\n";
        }

        std::cout << "Select mode index: ";
        size_t mode_index;
        std::cin >> mode_index;
        if (mode_index >= modes.size()) {
            std::cerr << "Invalid mode index\n";
            return 1;
        }

        DisplayMode mode = modes[mode_index];
        std::cout << "Selected mode: " << mode.getWidth() << "x" << mode.getHeight()
                  << " @ " << mode.getFreq() << " Hz\n";

        // 4. Select CRTC
        drmModeRes* resources = drmModeGetResources(drm.getFd());
        if (!resources || resources->count_crtcs == 0) {
            std::cerr << "Failed to get DRM resources or no CRTCs available\n";
            return 1;
        }

        std::cout << "Available CRTCs:\n";
        for (int i = 0; i < resources->count_crtcs; ++i) {
            std::cout << " [" << i << "] CRTC ID: " << resources->crtcs[i] << "\n";
        }

        std::cout << "Select CRTC index: ";
        int crtc_choice;
        std::cin >> crtc_choice;
        if (crtc_choice < 0 || crtc_choice >= resources->count_crtcs) {
            std::cerr << "Invalid CRTC index\n";
            drmModeFreeResources(resources);
            return 1;
        }

        uint32_t crtc_id = resources->crtcs[crtc_choice];
        drmModeFreeResources(resources);
        std::cout << "Selected CRTC ID: " << crtc_id << "\n";

        // 5. Create framebuffer
        DrmFramebuffer fb(drm, mode);
        std::cout << "Framebuffer created, id: " << fb.getId() << "\n";

        // 6. Set CRTC
        DrmCrtc crtc(drm, crtc_id);
        crtc.setCrtc(conn, mode, fb);
        std::cout << "CRTC set successfully\n";

        // 7. Draw test pattern
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

        // Wait before exit
        std::cout << "Press Enter to exit and restore previous mode...\n";
        std::cin.ignore(); // clear leftover newline
        std::cin.get();

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
