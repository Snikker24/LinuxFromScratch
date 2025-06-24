#include "drm_dri.h"

using namespace std;


int index(int a, int b, std::string msg){

    if(a==b)
        return a;

    if(a>b)
        swap(a,b);

    int idx;
    do{

        std::cout<<msg;
        std::cin>>idx;

    }while(idx<a||idx>=b);


    return idx;

}

int main()
{

    std::vector<std::unique_ptr<DrmDevice>> devices=DrmDevice::fetchAll();
    std::cout<<"Choose DRM device:\n";
    for(int i=0;i<devices.size();i++){

        std::cout<<"["<<i<<"]: "<<devices[i]->devicePath()<<"\n";

    }

    int idx=index(0,devices.size(),"Device index: ");

    DrmDevice& device=*devices[idx];
    std::vector<DrmDevice::DrmConnector> connectors=device.connectors();
    std::cout<<"Choose DRM device connector:\n";
    for(int i=0;i<connectors.size();i++){

        std::cout<<"["<<i<<"]: connector-id="<<connectors[i].id()<<"\n";

    }

    idx=index(0,connectors.size(),"Connector index: ");

    DrmDevice::DrmConnector connector=connectors[idx];
    std::vector<DrmDevice::DrmConnector::DisplayMode> modes=connector.modes();
    std::cout<<"Choose DRM connector mode:\n";
    for(int i=0;i<connectors.size();i++){

        std::cout<<"["<<i<<"]: "<<modes[i].width()<<"x"<<modes[i].height()<<"@"<<modes[i].frequency()<<"\n";

    }

    idx=index(0,modes.size(),"Mode index: ");

}
