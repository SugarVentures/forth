#ifndef ANDROID_VIDEO_ENGINE_HPP
#define ANDROID_VIDEO_ENGINE_HPP

#include <jni.h>

#include "video_engine.hpp"

namespace oppvs {
	class AndroidVideoEngine : public VideoEngine 
	{
	public:
		AndroidVideoEngine(frame_callback cb, void* user);

		static void initialize(JNIEnv* env);
		static void deinitialize();
	};
} // oppvs

#endif // ANDROID_VIDEO_ENGINE_HPP
