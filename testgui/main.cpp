#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <algorithm>

extern "C" {
    #include <xf86drm.h>
    #include <xf86drmMode.h>
}

// Utility for safe close
class DrmDevice {

public:
    DrmDevice(const std::string& path) {
        fd = open(path.c_str(), O_RDWR | O_CLOEXEC);
        if (fd < 0) {
            throw std::runtime_error("Failed to open DRM device: " + path);
        }
        this->path = path;

        this->conn_count=0;
        res=drmModeGetResources(fd);
        if (!res) {
            std::cerr << "drmModeGetResources failed\n";
        }else{
            this->conn_count=res->count_connectors;
            drmModeFreeResources(res);
        }

    }

    ~DrmDevice() {
        if (fd >= 0) close(fd);
    }

    int getFd() const { return fd; }
    std::string getPath() const { return path; }

    static std::vector<DrmDevice*> fetchAll(){

        std::vector<DrmDevice*> devices;
        DIR* dir = opendir("/dev/dri");
        if (!dir) return devices;

        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (strncmp(entry->d_name, "card", 4) == 0) {
                devices.emplace_back(new DrmDevice("/dev/dri/" + std::string(entry->d_name)));
            }
        }

        closedir(dir);
        return devices;
    }

    class Connector{


    private:

        int fd, conn_id, drm_supp;
        std::string parent_path;
        drmModeConnector* conn;

        Connector(DrmDevice drm_d,conn_id){


            drm_supp=0;
            fd=drm_d.getFd();
            parent_path=drm_d.getPath()
            conn=drmModeGetConnector(fd, conn_id)
            if (conn && conn->connection == DRM_MODE_CONNECTED && conn->count_modes > 0) {
                drm_supp=1;
            } else if(conn) {
                drmModeFreeConnector(conn);
            }

        }

    public:

        static int* resolutions(){



        }



    }

private:
    int fd, conn_count;
    std::string path;
    drmModeRes* res;
};

// Simple drawing function: fills screen with a color gradient
void draw_gradient(uint32_t* fb_ptr, uint32_t width, uint32_t height, uint32_t pitch) {
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            uint8_t r = x * 255 / width;
            uint8_t g = y * 255 / height;
            uint8_t b = 128;
            fb_ptr[y * (pitch / 4) + x] = (r << 16) | (g << 8) | b;
        }
    }
}

int main() {
    try {
        std::vector<DrmDevice*> drm_devices = DrmDevice::fetchAll();
        if (drm_devices.empty()) {
            std::cerr << "No DRM devices found\n";
            return 1;
        }

        std::cout << "Available DRM devices:\n";
        for (size_t i = 0; i < drm_devices.size(); ++i) {
            std::cout << i << ": " << drm_devices[i].getPath() << "\n";
        }

        std::cout << "Select device index: ";
        int choice;
        std::cin >> choice;
        if (choice < 0 || static_cast<size_t>(choice) >= drm_devices.size()) {
            std::cerr << "Invalid choice\n";
            return 1;
        }

        DrmDevice drm(drm_devices[choice]);
        int fd = drm.getFd();

        drmModeRes* res = drmModeGetResources(fd);
        if (!res) {
            std::cerr << "drmModeGetResources failed\n";
            return 1;
        }

        std::vector<drmModeConnector*> connectors;
        for (int i = 0; i < res->count_connectors; ++i) {
            drmModeConnector* conn = drmModeGetConnector(fd, res->connectors[i]);
            if (conn && conn->connection == DRM_MODE_CONNECTED && conn->count_modes > 0) {
                connectors.push_back(conn);
            } else if (conn) {
                drmModeFreeConnector(conn);
            }
        }

        if (connectors.empty()) {
            std::cerr << "No connected displays found\n";
            drmModeFreeResources(res);
            return 1;
        }

        std::cout << "Available displays and resolutions:\n";
        for (size_t i = 0; i < connectors.size(); ++i) {
            std::cout << i << ": Connector ID: " << connectors[i]->connector_id << ", Modes: ";
            for (int m = 0; m < connectors[i]->count_modes; ++m) {
                std::cout << "(" << connectors[i]->modes[m].hdisplay << "x" << connectors[i]->modes[m].vdisplay << ") ";
            }
            std::cout << "\n";
        }

        std::cout << "Select connector index: ";
        int conn_choice;
        std::cin >> conn_choice;
        if (conn_choice < 0 || static_cast<size_t>(conn_choice) >= connectors.size()) {
            std::cerr << "Invalid connector index\n";
            return 1;
        }

        drmModeConnector* conn = connectors[conn_choice];

        std::cout << "Available modes for connector " << conn->connector_id << ":\n";
        for (int i = 0; i < conn->count_modes; ++i) {
            std::cout << i << ": " << conn->modes[i].hdisplay << "x" << conn->modes[i].vdisplay << "\n";
        }

        std::cout << "Select mode index: ";
        int mode_choice;
        std::cin >> mode_choice;
        if (mode_choice < 0 || mode_choice >= conn->count_modes) {
            std::cerr << "Invalid mode index\n";
            return 1;
        }

        drmModeModeInfo mode = conn->modes[mode_choice];

        drmModeEncoder* enc = drmModeGetEncoder(fd, conn->encoder_id);
        uint32_t crtc_id = enc->crtc_id;
        drmModeFreeEncoder(enc);

        struct drm_mode_create_dumb create = {};
        create.width = mode.hdisplay;
        create.height = mode.vdisplay;
        create.bpp = 32;

        if (ioctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &create) < 0) {
            std::cerr << "DRM_IOCTL_MODE_CREATE_DUMB failed\n";
            return 1;
        }

        uint32_t fb;
        if (drmModeAddFB(fd, create.width, create.height, 24, 32,
                         create.pitch, create.handle, &fb)) {
            std::cerr << "drmModeAddFB failed\n";
            return 1;
        }

        struct drm_mode_map_dumb map = {};
        map.handle = create.handle;
        if (ioctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &map) < 0) {
            std::cerr << "DRM_IOCTL_MODE_MAP_DUMB failed\n";
            return 1;
        }

        void* fb_ptr_void = mmap(0, create.size, PROT_READ | PROT_WRITE, MAP_SHARED,
                                 fd, map.offset);
        if (fb_ptr_void == MAP_FAILED) {
            std::cerr << "mmap failed\n";
            return 1;
        }

        draw_gradient(static_cast<uint32_t*>(fb_ptr_void), create.width, create.height, create.pitch);

        if (drmModeSetCrtc(fd, crtc_id, fb, 0, 0, &conn->connector_id, 1, &mode)) {
            std::cerr << "drmModeSetCrtc failed\n";
            return 1;
        }

        std::cout << "Gradient drawn. Press Enter to exit...\n";
        std::cin.ignore();
        std::cin.get();

        munmap(fb_ptr_void, create.size);
        drmModeRmFB(fd, fb);

        struct drm_mode_destroy_dumb destroy = {};
        destroy.handle = create.handle;
        ioctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy);

        for (auto* c : connectors) drmModeFreeConnector(c);
        drmModeFreeResources(res);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
