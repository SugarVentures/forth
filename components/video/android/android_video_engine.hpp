#ifndef ANDROID_VIDEO_ENGINE_HPP
#define ANDROID_VIDEO_ENGINE_HPP

#include <jni.h>

#include "video_engine.hpp"

namespace oppvs {
	class AndroidVideoEngine : public VideoEngine 
	{
	public:
		AndroidVideoEngine(frame_callback cb, void* user);

		static void initialize(JNIEnv* env, jobject context);
		static void deinitialize();

		void setupCaptureSession(VideoActiveSource* source);
		void startCaptureSession();
		void runCaptureSession(jobject intent);
	private:
		jclass j_screen_recorder_class;
		jobject j_screen_recorder;

	};
} // oppvs

#endif // ANDROID_VIDEO_ENGINE_HPP
