#include <jni.h>

#include "forth_broadcaster.h"
//
// Created by Cao Minh Trang on 3/9/16.
//

using namespace oppvs;

ForthBroadcaster* globalBroadcasterEngine;

extern "C" {
    JNIEXPORT void JNICALL Java_tv_forth_broadcaster_ForthBroadcasterWrapper_Initialize(JNIEnv* env, jobject obj, jobject context);
    JNIEXPORT void JNICALL Java_tv_forth_broadcaster_ForthBroadcasterWrapper_setupCaptureSession(JNIEnv* env, jobject obj, jobject wrapper);
    JNIEXPORT int JNICALL Java_tv_forth_broadcaster_ForthBroadcasterWrapper_startCaptureSession(JNIEnv* env, jobject obj);
    JNIEXPORT void JNICALL Java_tv_forth_broadcaster_ForthBroadcasterWrapper_runCaptureSession(JNIEnv* env, jobject obj, jobject intent);
    JNIEXPORT void JNICALL Java_tv_forth_broadcaster_ForthBroadcasterWrapper_pushVideoFrame(JNIEnv* env, jobject obj, jobject frame, jint width, jint height, jint stride);
    JNIEXPORT void JNICALL Java_tv_forth_broadcaster_ForthBroadcasterWrapper_startStreaming(JNIEnv* env, jobject obj, jstring streamKey, jstring serverAddress);
};

JNIEXPORT void JNICALL Java_tv_forth_broadcaster_ForthBroadcasterWrapper_Initialize(JNIEnv* env, jobject obj, jobject context) {
    ForthBroadcaster::Initialize(env, env->NewGlobalRef(context));
    globalBroadcasterEngine = new ForthBroadcaster();
}

JNIEXPORT void JNICALL Java_tv_forth_broadcaster_ForthBroadcasterWrapper_setupCaptureSession(JNIEnv* env, jobject obj, jobject wrapper) {
    globalBroadcasterEngine->setupCaptureSession(wrapper);
}

JNIEXPORT int JNICALL Java_tv_forth_broadcaster_ForthBroadcasterWrapper_startCaptureSession(JNIEnv* env, jobject obj) {
    return globalBroadcasterEngine->startCaptureSession(0, 0);
}

JNIEXPORT void JNICALL Java_tv_forth_broadcaster_ForthBroadcasterWrapper_runCaptureSession(JNIEnv* env, jobject obj, jobject intent) {
    globalBroadcasterEngine->runCaptureSession(intent);
}

JNIEXPORT void JNICALL Java_tv_forth_broadcaster_ForthBroadcasterWrapper_pushVideoFrame(JNIEnv* env, jobject obj, jobject frame, jint width, jint height, jint stride) {
    globalBroadcasterEngine->pushVideoFrame(env->GetDirectBufferAddress(frame), width, height, stride);
}

JNIEXPORT void JNICALL Java_tv_forth_broadcaster_ForthBroadcasterWrapper_startStreaming(JNIEnv* env, jobject obj, jstring streamKey, jstring serverAddress) {
    const char *nativeStreamKey = env->GetStringUTFChars(streamKey, JNI_FALSE);
    const char *nativeServerAddress = env->GetStringUTFChars(serverAddress, JNI_FALSE);
    globalBroadcasterEngine->startStreaming(nativeStreamKey, nativeServerAddress);
}