#include "ios_video_engine.hpp"

namespace oppvs {
    IosVideoEngine::IosVideoEngine(frame_callback cb, void* user) : VideoEngine(cb, user)
    {
        info = oppvs_vc_info_alloc();
    }
    
    void IosVideoEngine::getListCaptureDevices(std::vector<VideoCaptureDevice>& result) {
        oppvs_get_list_video_sources(info, result);
    }
    
    void IosVideoEngine::setupCaptureSession(VideoActiveSource* source)
    {
        IosVideoCapture* videocap = new IosVideoCapture(callback_frame, source->user, source);
        videocap->setup();
        source->capture = (VideoCapture*)videocap;
    }
    
    int IosVideoEngine::startCaptureSession(VideoActiveSource& source)
    {
        return source.capture->start();
    }
    
    void IosVideoEngine::stopCaptureSession(const VideoActiveSource& source)
    {
        source.capture->stop();
    }
    
    void IosVideoEngine::updateConfiguration(const VideoActiveSource& source)
    {
        source.capture->updateConfiguration(source);
    }

} // oppvs