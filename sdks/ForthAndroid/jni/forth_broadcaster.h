#ifndef FORTH_BROADCASTER_H
#define FORTH_BROADCASTER_H

#include <jni.h>
#include <string>

namespace oppvs {
	struct ForthBroadcastObject;

	class ForthBroadcaster
	{
	public:
		ForthBroadcaster();
		~ForthBroadcaster();

		static void Initialize(JNIEnv* env, jobject context);
		void setupCaptureSession(jobject wrapperObject);
		int startCaptureSession(int deviceIndex, int type);
		void runCaptureSession(jobject intent);
		void pushVideoFrame(void* data, jint width, jint height, jint stride);

		void startStreaming(const std::string& streamKey, const std::string& serverAddress);

	private:
		ForthBroadcastObject* m_object;
		bool m_isSetPixelBuffer;


	};
} // oppvs

#endif // FORTH_BROADCASTER_H
