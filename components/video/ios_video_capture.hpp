#ifndef OPPVS_IOS_VIDEO_CAPTURE_HPP
#define OPPVS_IOS_VIDEO_CAPTURE_HPP

#include "video_capture.hpp"
#include "ios_video_capture_interface.h"

namespace oppvs {
	class IosVideoCapture : public VideoCapture
	{
	public:
		IosVideoCapture(frame_callback cbf, void* user, VideoActiveSource* source);
		~IosVideoCapture();

		void setup();
        void start();
        void stop();
        void* getSession();
	};
} // oppvs

#endif // OPPVS_IOS_VIDEO_CAPTURE_HPP
