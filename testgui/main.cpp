#include <iostream>
#include <memory>
#include "DrmDevice.h"
#include "DrmConnector.h"
#include "DrmCrtc.h"
#include "DrmFramebuffer.h"
#include "DisplayMode.h"

void drawGradient(uint32_t* buffer, uint32_t width, uint32_t height, uint32_t pitch) {
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            uint8_t r = x * 255 / width;
            uint8_t g = y * 255 / height;
            uint8_t b = 128;
            buffer[y * (pitch / 4) + x] = (r << 16) | (g << 8) | b;
        }
    }
}

int main() {
    try {
        auto devices = DrmDevice::enumerate();
        if (devices.empty()) {
            std::cerr << "No DRM devices found.\n";
            return 1;
        }

        std::cout << "Available DRM devices:\n";
        for (size_t i = 0; i < devices.size(); ++i) {
            std::cout << i << ": " << devices[i]->getPath() << "\n";
        }

        std::cout << "Select device index: ";
        size_t devIndex;
        std::cin >> devIndex;

        if (devIndex >= devices.size()) {
            std::cerr << "Invalid device index.\n";
            return 1;
        }

        DrmDevice& device = *devices[devIndex];
        auto connectors = device.getConnectors();

        std::cout << "Available connectors:\n";
        for (size_t i = 0; i < connectors.size(); ++i) {
            std::cout << i << ": Connector ID = " << connectors[i]->getId()
                      << " (" << connectors[i]->getModeCount() << " modes)\n";
        }

        std::cout << "Select connector index: ";
        size_t connIndex;
        std::cin >> connIndex;

        if (connIndex >= connectors.size()) {
            std::cerr << "Invalid connector index.\n";
            return 1;
        }

        DrmConnector& connector = *connectors[connIndex];
        auto modes = connector.getModes();

        std::cout << "Available modes:\n";
        for (size_t i = 0; i < modes.size(); ++i) {
            std::cout << i << ": " << modes[i].getWidth() << "x" << modes[i].getHeight()
                      << " @ " << modes[i].getFreq() << "Hz\n";
        }

        std::cout << "Select mode index: ";
        size_t modeIndex;
        std::cin >> modeIndex;

        if (modeIndex >= modes.size()) {
            std::cerr << "Invalid mode index.\n";
            return 1;
        }

        DisplayMode& mode = modes[modeIndex];
        DrmCrtc crtc(device.getFd(), connector.getEncoder()->crtc_id, mode.getRaw());
        DrmFramebuffer fb(device.getFd(), mode.getWidth(), mode.getHeight());

        drawGradient(reinterpret_cast<uint32_t*>(fb.getBufferPtr()),
                     fb.getWidth(), fb.getHeight(), fb.getPitch());

        if (!crtc.setFramebuffer(fb.getFbId(), connector.getId())) {
            std::cerr << "Failed to set CRTC.\n";
            return 1;
        }

        std::cout << "Gradient displayed. Press Enter to exit...\n";
        std::cin.ignore();
        std::cin.get();

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
