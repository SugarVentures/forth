#include "android_video_engine.hpp"
#include "logs.h"

jobject applicationContext = nullptr;
JavaVM* javaVM = nullptr;

namespace oppvs {
	AndroidVideoEngine::AndroidVideoEngine(frame_callback cb, void* user)
	{

	}

	void AndroidVideoEngine::initialize(JNIEnv* env, jobject context)
	{
		LOGD("Initialize");
		if (applicationContext)
		{
			env->DeleteGlobalRef(applicationContext);
		}
		applicationContext = env->NewGlobalRef(context);
		env->GetJavaVM(&javaVM);

		jclass j_info_class = env->FindClass("tv/forth/videoengine/AndroidVideoSourceInfo");
		assert(j_info_class);

		jmethodID j_list_video_sources = env->GetStaticMethodID(j_info_class, "getListCaptureDevices", "(Ljava/lang/Object;)V");
		env->CallStaticVoidMethod(j_info_class, j_list_video_sources, context);

		
	}

	void AndroidVideoEngine::setupCaptureSession(VideoActiveSource* source)
	{
		if (!source)
			return;

		JNIEnv *env;
		javaVM->AttachCurrentThread(&env, NULL);
		jclass j_screen_recorder_class_local = env->FindClass("tv/forth/videoengine/ScreenRecorder");

		jobject wrapperObject = env->NewGlobalRef((jobject)source->user);

		
		jmethodID j_screen_recorder_constructor = env->GetMethodID(j_screen_recorder_class_local, "<init>", "(Landroid/content/Context;Ltv/forth/broadcaster/ForthBroadcasterWrapper;)V");
		jobject j_screen_recorder_local = env->NewObject(j_screen_recorder_class_local, j_screen_recorder_constructor, applicationContext, wrapperObject);

		jmethodID j_initialize = env->GetMethodID(j_screen_recorder_class_local, "initialize", "()V");
		env->CallVoidMethod(j_screen_recorder_local, j_initialize);

		j_screen_recorder_class = reinterpret_cast<jclass>(env->NewGlobalRef(j_screen_recorder_class_local));
		env->DeleteLocalRef(j_screen_recorder_class_local);
		j_screen_recorder = reinterpret_cast<jobject>(env->NewGlobalRef(j_screen_recorder_local));
	}

	void AndroidVideoEngine::startCaptureSession() {
		JNIEnv *env;
		javaVM->AttachCurrentThread(&env, NULL);
		jmethodID j_start = env->GetMethodID(j_screen_recorder_class, "start", "()V");
		env->CallVoidMethod(j_screen_recorder, j_start);
	}

	void AndroidVideoEngine::runCaptureSession(jobject intent)
	{
		JNIEnv *env;
		javaVM->AttachCurrentThread(&env, NULL);
		jmethodID j_onActivityOnResult = env->GetMethodID(j_screen_recorder_class, "onActivityOnResult", "(Landroid/content/Intent;)V");
		env->CallVoidMethod(j_screen_recorder, j_onActivityOnResult, intent);
	}

} // oppvs