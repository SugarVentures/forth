LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE	:= forthsdk
LOCAL_SRC_FILES	:= forth-jni.cpp ../../../components/thread/thread.cpp

TARGET_PLATFORM := android-16

LOCAL_LDLIBS	+= -std=gnu++11 -llog 

LOCAL_CFLAGS	+= -Wall

LOCAL_C_INCLUDES += ../../components/include
LOCAL_C_INCLUDES += ../../components/thread

include $(BUILD_SHARED_LIBRARY)