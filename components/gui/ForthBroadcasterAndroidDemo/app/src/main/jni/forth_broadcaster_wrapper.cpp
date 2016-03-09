#include <jni.h>

#include "forth_broadcaster.h"
//
// Created by Cao Minh Trang on 3/9/16.
//

using namespace oppvs;

extern "C" {
    JNIEXPORT void JNICALL Java_tv_forth_broadcaster_ForthBroadcasterWrapper_Initialize(JNIEnv* env, jobject obj);
};

JNIEXPORT void JNICALL Java_tv_forth_broadcaster_ForthBroadcasterWrapper_Initialize(JNIEnv* env, jobject obj) {
    ForthBroadcaster::Initialize(env);
}
