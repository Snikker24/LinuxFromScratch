#include "drm_dri.h"

using namespace std;

int main()
{

    std::vector<DrmDevice> devices=DrmDevice::fetchAll();
    std::cout<<"Choose DRM device:\n";
    for(int i=0;i<devices.size();i++){

        std::cout<<"["<<i<<"]: "<<devices[i].devicePath()<<"\n";

    }

    int idx;
    do{

        std::cout<<"Device index:";
        std::cin>>idx;

    }while(idx<0||idx>=devices.size());



}
