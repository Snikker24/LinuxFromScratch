#include "drm_dri.h"
#include <atomic>
#include <cmath>
#include <thread>
#include <chrono>

class Renderer{

private:
    Framebuffer* fb;
    std::atomic<bool> status;
    std::thread r_t;

    static void renderLoop(Framebuffer * fb, std::atomic<bool>* status){

        int width = fb->mode().width();
        int height = fb->mode().height();
        int pitch = fb->bytesize() / height;

        while (*status) {

        fb->render();
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
        }
        std::cout<<"Exited";

        fb->clearMem();
        fb->render();

    }

public:

    Renderer(Framebuffer& a_fb){

        fb=&a_fb;
        status=false;
    }


    void start(){

        status=true;
        r_t=std::thread(Renderer::renderLoop, fb, &status);

    }

    void stop(){

        status=false;
        r_t.join();
    }

    bool running(){

        return status;

    }

};

static void testpattern0(Framebuffer * fb){

    uint8_t* pixels = fb->pixels();
    int width = fb->mode().width();
    int height = fb->mode().height();
    int pitch = fb->bytesize() / height; // assumes evenly divisible

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int offset = y * pitch + x * 4;
            pixels[offset + 0] = x * 255 / width;  // Blue
            pixels[offset + 1] = y * 255 / height; // Green
            pixels[offset + 2] = 128;              // Red
            pixels[offset + 3] = 0;                // Alpha (unused)
        }
    }

}

void testpattern1(Framebuffer * fb){

    uint8_t* pixels = fb->pixels();
    int width = fb->mode().width();
    int height = fb->mode().height();
    int pitch = fb->bytesize() / height; // assumes evenly divisible

    float square_size=width*height*0.25f;

    for (int y = 0; y < height; ++y) {
        int square_y = static_cast<int>(std::floor(y / square_size));

        for (int x = 0; x < width; ++x) {
            int square_x = static_cast<int>(std::floor(x / square_size));
            bool is_violet = (square_x + square_y) % 2 == 0;
            int offset = y * pitch + x * 4;

            if (is_violet) {
                pixels[offset + 0] = 255; // Blue
                pixels[offset + 1] = 0;   // Green
                pixels[offset + 2] = 255; // Red
                pixels[offset + 3] = 0;   // Alpha
            } else {
                pixels[offset + 0] = 0;
                pixels[offset + 1] = 0;
                pixels[offset + 2] = 0;
                pixels[offset + 3] = 0;
            }
        }
    }
}

void testpattern_cycle(Framebuffer * fb, bool* status){

    while(*status){

        testpattern0(fb);
        std::this_thread::sleep_for(std::chrono::seconds(2));
        testpattern1(fb);
        std::this_thread::sleep_for(std::chrono::seconds(2));

    }


}

int index(int a, int b, std::string msg){
    if(a == b)
        return a;
    if(a > b)
        std::swap(a,b);

    int idx;
    do {
        std::cout << msg;
        std::cin >> idx;
    } while(idx < a || idx >= b);

    return idx;
}

int main(int argc, char* argv[])
{
    std::vector<DrmDevice> devices = DrmDevice::fetchAll();
    std::cout << "Choose DRM device:\n";
    for(int i = 0; i < devices.size(); i++) {
        std::cout << "[" << i << "]: " << devices[i].path() << "\n";
    }

    int idx = index(0, devices.size(), "Device index: ");
    DrmDevice device(devices[idx].path());

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

    bool cycle=false;
    std::thread thd;
    if(argc==2){

        std::string arg1=argv[1];

        if(arg1=="testpattern0")
            testpattern0(&fb);
        else if(arg1=="testpattern1")
            testpattern1(&fb);
        else if(arg1=="testpatterncycle"){
            cycle=true;
            thd=std::thread(testpattern_cycle,&fb,&cycle);
        }

    }else{

        testpattern0(&fb);

    }

    std::cout << "Framebuffer active. Press Enter to exit...\n";



    Renderer ren(fb);
    ren.start();

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();

    cycle=false;
    thd.join();
    std::cout << "Closing thread now...\n";

    ren.stop();

    return 0;
}
