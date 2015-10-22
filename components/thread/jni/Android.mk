LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE	:= threadengine
LOCAL_SRC_FILES	:= threadengine-jni.cpp ../thread.cpp

TARGET_PLATFORM := android-16

LOCAL_LDLIBS	+= -std=gnu++11 -llog 

LOCAL_CFLAGS	+= -Wall

include $(BUILD_SHARED_LIBRARY)