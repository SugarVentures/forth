#include "ios_video_engine.hpp"

namespace oppvs {
    IosVideoEngine::IosVideoEngine(frame_callback cb, void* user) : VideoEngine(cb, user)
    {
        printf("Init video engine\n");
        info = oppvs_vc_info_alloc();
    }
    
    void IosVideoEngine::getListCaptureDevices(std::vector<VideoCaptureDevice>& result) {
        oppvs_get_list_video_sources(info, result);
    }

} // oppvs