#include <jni.h>


#include "forth_viewer.h"
#include "forth_renderer.h"

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/log.h>
#define LIB_PUBLIC __attribute__ ((visibility ("default"))) 

using namespace oppvs;

ForthRenderer* fRenderer;
JavaVM* javaVM = NULL;
jobject javaObject;

static jfieldID gContext_EGLContextFieldID;
static jfieldID gDisplay_EGLDisplayFieldID;
static jfieldID gConfig_EGLConfigFieldID;

static jclass gConfig_class;
static jmethodID gConfig_ctorID;

static jobject eglDisplay;
static jobject sharedContext;
static jobject eglConfig;
static bool isContextCreated = false;

static inline bool checkFieldExist(JNIEnv* env, jfieldID* fid, jclass oclass, const char* fname, const char* type)
{
	env->ExceptionClear();
	*fid = env->GetFieldID(oclass, fname, type);
	if (JNI_TRUE == env->ExceptionCheck())
	{
		env->ExceptionClear();
		return false;
	}
	return true;
}

static inline EGLContext getContext(JNIEnv* env, jobject o) {
	if (!o) return EGL_NO_CONTEXT;

	jclass context_class = env->FindClass("com/google/android/gles_jni/EGLContextImpl");
	if (env->ExceptionCheck()) {
		__android_log_print(ANDROID_LOG_DEBUG, "oppvs", "no class found");
		return EGL_NO_CONTEXT;
	}

	if (checkFieldExist(env, &gContext_EGLContextFieldID, context_class, "mEGLContext", "I")) {
		return (EGLContext) env->GetIntField(o, gContext_EGLContextFieldID);
	}
	if (checkFieldExist(env, &gContext_EGLContextFieldID, context_class, "mEGLContext", "J"))
		return (EGLContext)env->GetLongField(o, gContext_EGLContextFieldID);
	return EGL_NO_CONTEXT;
}

static inline EGLConfig getConfig(JNIEnv* env, jobject o) {
	if (!o) return 0;
	jclass config_class = env->FindClass("com/google/android/gles_jni/EGLConfigImpl");

	if (checkFieldExist(env, &gConfig_EGLConfigFieldID, config_class, "mEGLConfig", "I")) {
		return (EGLConfig) env->GetIntField(o, gConfig_EGLConfigFieldID);
	}
	if (checkFieldExist(env, &gConfig_EGLConfigFieldID, config_class, "mEGLConfig", "J"))
		return (EGLConfig)env->GetLongField(o, gConfig_EGLConfigFieldID);
	return 0;
}

static inline EGLDisplay getDisplay(JNIEnv* env, jobject o) {
	if (!o) return EGL_NO_DISPLAY;
	jclass display_class = env->FindClass("com/google/android/gles_jni/EGLDisplayImpl");

	if (checkFieldExist(env, &gDisplay_EGLDisplayFieldID, display_class, "mEGLDisplay", "I"))
		return (EGLDisplay)env->GetIntField(o, gDisplay_EGLDisplayFieldID);
	if (checkFieldExist(env, &gDisplay_EGLDisplayFieldID, display_class, "mEGLDisplay", "J"))
		return (EGLDisplay)env->GetLongField(o, gDisplay_EGLDisplayFieldID);
	return EGL_NO_DISPLAY;
}

void callback()
{
	/*JNIEnv *env;
	javaVM->AttachCurrentThread(&env, NULL);

	jclass clazz = env->GetObjectClass(javaObject);
	if (!clazz) {
		__android_log_print(ANDROID_LOG_DEBUG, "oppvs", "no class found");
		return;
	}
	jmethodID messageMe = env->GetMethodID(clazz, "frameCallback", "()V");
	if (!messageMe){
		__android_log_print(ANDROID_LOG_DEBUG, "oppvs", "no method found");
		return;
	}*/

	JNIEnv *env;
	javaVM->AttachCurrentThread(&env, NULL);
	if (!isContextCreated) {
		if (fRenderer) {
			fRenderer->setContext(getContext(env, sharedContext), getDisplay(env, eglDisplay),
								  getConfig(env, eglConfig));
		}
		isContextCreated = true;
	}
	//env->CallVoidMethod(javaObject, messageMe);
}

extern "C" LIB_PUBLIC int Java_tv_forth_forthviewerandroiddemo_ForthViewer_startStreaming(JNIEnv* env, jobject thiz, jstring streamKey, jstring serverAddress)
{
	env->GetJavaVM(&javaVM);
	javaObject = env->NewGlobalRef(thiz);

	ForthViewer viewer;
	const char *nativeStreamKey = env->GetStringUTFChars(streamKey, JNI_FALSE);
	const char *nativeServerAddress = env->GetStringUTFChars(serverAddress, JNI_FALSE);
	viewer.attachRenderer(fRenderer);
	viewer.attachCallback(callback);
	viewer.startStreaming(nativeStreamKey, nativeServerAddress);
	return 1;
}

extern "C" {
	JNIEXPORT int JNICALL Java_tv_forth_forthviewerandroiddemo_ForthViewer_init(JNIEnv * env, jobject obj);
	JNIEXPORT int JNICALL Java_tv_forth_forthviewerandroiddemo_ForthViewer_setup(JNIEnv * env, jobject obj,  jint width, jint height);
	JNIEXPORT void JNICALL Java_tv_forth_forthviewerandroiddemo_ForthViewer_render(JNIEnv * env, jobject obj);
	JNIEXPORT void JNICALL Java_tv_forth_forthviewerandroiddemo_ForthViewer_setContext(JNIEnv* env, jobject obj, jobject context, jobject display, jobject config);
};

JNIEXPORT int JNICALL Java_tv_forth_forthviewerandroiddemo_ForthViewer_init(JNIEnv * env, jobject obj)
{
	fRenderer = new ForthRenderer();
	int ret = fRenderer->init();
	return ret;
}

JNIEXPORT int JNICALL Java_tv_forth_forthviewerandroiddemo_ForthViewer_setup(JNIEnv * env, jobject obj,  jint width, jint height)
{
	int ret = 0;
	if (fRenderer)
		ret = fRenderer->setup(width, height);
	return ret;
}

JNIEXPORT void JNICALL Java_tv_forth_forthviewerandroiddemo_ForthViewer_render(JNIEnv * env, jobject obj)
{
	if (fRenderer)
		fRenderer->render();
}


JNIEXPORT void JNICALL Java_tv_forth_forthviewerandroiddemo_ForthViewer_setContext(JNIEnv * env, jobject obj, jobject context, jobject display, jobject config)
{
	sharedContext = env->NewGlobalRef(context);
	eglDisplay = env->NewGlobalRef(display);
	eglConfig = env->NewGlobalRef(config);
}
