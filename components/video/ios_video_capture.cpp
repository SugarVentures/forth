#include "ios_video_capture.hpp"

namespace oppvs {
    IosVideoCapture::IosVideoCapture(frame_callback cbf, void* user, VideoActiveSource* source) :
    VideoCapture(cbf, user, source)
	{
        printf("Init video capture\n");
        m_cap = oppvs_vc_av_alloc();
        oppvs_av_set_callback(m_cap, cbf, user);
	}

	IosVideoCapture::~IosVideoCapture()
	{

	}

	void IosVideoCapture::setup()
	{
        oppvs_setup_capture_session(m_cap, m_source);
	}
    
    int IosVideoCapture::start()
    {
        return oppvs_start_video_recording(m_cap);
    }
    
    void IosVideoCapture::stop()
    {
        oppvs_stop_video_recording(m_cap);
    }
    
    void IosVideoCapture::updateConfiguration(const VideoActiveSource& source)
    {
        oppvs_update_configuration(m_cap, source);
    }
} // oppvs