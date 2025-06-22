// Pure KMS mode-set example using libdrm and dumb buffers
// Cross-GPU, vendor-agnostic, no GPU acceleration

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm/drm_mode.h>

int main() {
    int drm_fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
    if (drm_fd < 0) {
        perror("open /dev/dri/card0");
        return 1;
    }

    drmModeRes *resources = drmModeGetResources(drm_fd);
    if (!resources) {
        perror("drmModeGetResources");
        close(drm_fd);
        return 1;
    }

    drmModeConnector *connector = NULL;
    drmModeModeInfo *modes = NULL;
    uint32_t conn_id = 0;

    for (int i = 0; i < resources->count_connectors; ++i) {
        connector = drmModeGetConnector(drm_fd, resources->connectors[i]);
        if (connector->connection == DRM_MODE_CONNECTED && connector->count_modes > 0) {
            conn_id = connector->connector_id;
            modes = connector->modes;
            break;
        }
        drmModeFreeConnector(connector);
    }

    if (!connector) {
        fprintf(stderr, "No connected monitor found.\n");
        drmModeFreeResources(resources);
        return 1;
    }

    printf("Available modes:\n");
    for (int i = 0; i < connector->count_modes; ++i) {
        printf(" [%d] %s (%dx%d)\n", i, modes[i].name, modes[i].hdisplay, modes[i].vdisplay);
    }

    int choice = 0;
    printf("Select mode index: ");
    scanf("%d", &choice);

    if (choice < 0 || choice >= connector->count_modes) {
        fprintf(stderr, "Invalid choice.\n");
        drmModeFreeConnector(connector);
        drmModeFreeResources(resources);
        return 1;
    }

    drmModeModeInfo mode = modes[choice];
    drmModeEncoder *encoder = drmModeGetEncoder(drm_fd, connector->encoder_id);
    drmModeCrtc *crtc = drmModeGetCrtc(drm_fd, encoder->crtc_id);

    // Create dumb buffer
    struct drm_mode_create_dumb create = {0};
    create.width = mode.hdisplay;
    create.height = mode.vdisplay;
    create.bpp = 32;
    ioctl(drm_fd, DRM_IOCTL_MODE_CREATE_DUMB, &create);

    uint32_t fb;
    struct drm_mode_fb_cmd fb_cmd = {
        .width = create.width,
        .height = create.height,
        .pitch = create.pitch,
        .bpp = create.bpp,
        .depth = 24,
        .handle = create.handle,
    };
    drmModeAddFB(drm_fd, fb_cmd.width, fb_cmd.height, fb_cmd.depth, fb_cmd.bpp,
                 fb_cmd.pitch, fb_cmd.handle, &fb);

    // Map memory for software rendering
    struct drm_mode_map_dumb map = { .handle = create.handle };
    ioctl(drm_fd, DRM_IOCTL_MODE_MAP_DUMB, &map);

    uint8_t *fb_ptr = mmap(0, create.size, PROT_READ | PROT_WRITE, MAP_SHARED,
                           drm_fd, map.offset);

    // Fill framebuffer with color bars (simple software rendering)
    for (uint32_t y = 0; y < create.height; y++) {
        for (uint32_t x = 0; x < create.width; x++) {
            uint32_t offset = y * create.pitch + x * 4;
            uint8_t r = (x * 255) / create.width;
            uint8_t g = (y * 255) / create.height;
            uint8_t b = 128;
            fb_ptr[offset + 0] = b;
            fb_ptr[offset + 1] = g;
            fb_ptr[offset + 2] = r;
            fb_ptr[offset + 3] = 0;
        }
    }

    // Set the CRTC to display the framebuffer
    drmModeSetCrtc(drm_fd, crtc->crtc_id, fb, 0, 0, &conn_id, 1, &mode);
    printf("Mode %s set successfully.\n", mode.name);

    printf("Press Enter to exit...");
    getchar(); getchar(); // Wait for user input before cleanup

    // Cleanup
    munmap(fb_ptr, create.size);
    drmModeRmFB(drm_fd, fb);
    struct drm_mode_destroy_dumb destroy = { .handle = create.handle };
    ioctl(drm_fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy);

    drmModeFreeCrtc(crtc);
    drmModeFreeEncoder(encoder);
    drmModeFreeConnector(connector);
    drmModeFreeResources(resources);
    close(drm_fd);
    return 0;
}
