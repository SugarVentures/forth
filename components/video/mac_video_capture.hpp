/*

*/

#pragma once

#include "video_engine.hpp"

namespace oppvs {

	class MacVideoCapture : public VideoCapture {
		public:
			MacVideoCapture(frame_callback fc, void* user, VideoActiveSource* source);
			~MacVideoCapture();

			void setup();
			int start();
			void stop();
			void updateConfiguration(const VideoActiveSource& source);
	};

	class MacVideoEngine : public VideoEngine {
	public:
		MacVideoEngine();
		MacVideoEngine(frame_callback fc, void* user);
		~MacVideoEngine();

		void getListCaptureDevices(std::vector<VideoCaptureDevice>& result);

		void getListVideoSource(std::vector<VideoScreenSource>& result);

		void getListMonitors(std::vector<Monitor>& result);

		void setupCaptureSessions();

		void setupCaptureSession(VideoActiveSource* source);
		int startCaptureSession(VideoActiveSource& source);

		void startRecording();

		void stopRecording();

		int getDeviceID(std::string& title);

		void updateConfiguration(const VideoActiveSource& source);
	};
}