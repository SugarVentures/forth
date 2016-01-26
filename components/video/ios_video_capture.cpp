#include "ios_video_capture.hpp"

namespace oppvs {
    IosVideoCapture::IosVideoCapture(frame_callback cbf, void* user, VideoActiveSource* source) :
    VideoCapture(cbf, user, source)
	{

	}

	IosVideoCapture::~IosVideoCapture()
	{

	}

	void IosVideoCapture::setup()
	{

	}
} // oppvs