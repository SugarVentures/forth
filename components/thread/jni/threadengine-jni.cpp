#include <jni.h>


#include "../thread.hpp"


#define LIB_PUBLIC __attribute__ ((visibility ("default"))) 

using namespace oppvs;

extern "C" LIB_PUBLIC jstring Java_com_example_caominhtrang_myapplication_ThreadEngine_getString(JNIEnv* env, jobject thiz)
{
	Thread* thread;
	thread = new Thread(Thread::defaultRun, NULL);
	thread->create();
	delete thread;

	return env->NewStringUTF("Hello");
}