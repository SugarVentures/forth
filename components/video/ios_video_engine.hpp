#ifndef OPPVS_IOS_VIDEO_ENGINE_HPP
#define OPPVS_IOS_VIDEO_ENGINE_HPP

#include "video_engine.hpp"
#include "ios_video_capture_interface.h"

namespace oppvs {
	class IosVideoEngine : public VideoEngine
	{
	public:
        IosVideoEngine(frame_callback cb, void* user);
		void getListCaptureDevices(std::vector<VideoCaptureDevice>& result);
        
        void setupCaptureSession(VideoActiveSource* source);
        void startCaptureSession(VideoActiveSource& source);
	};
} // oppvs

#endif // OPPVS_IOS_VIDEO_ENGINE_HPP
