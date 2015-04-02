/*

*/

#pragma once

#include "../interface/video_capture.hpp"

namespace oppvs {
	class MacVideoEngine : public VideoEngine {
	public:
		MacVideoEngine();
		MacVideoEngine(frame_callback fc, void* user);
		~MacVideoEngine();

		void getListCaptureDevices(std::vector<VideoCaptureDevice>& result);

		void getListVideoSource(std::vector<VideoScreenSource>& result);

		void setupCaptureSessions();

		void startRecording();

		void stopRecording();

		int getDeviceID(std::string& title);
	};
}