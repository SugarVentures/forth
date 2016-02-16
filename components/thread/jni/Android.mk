LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE	:= threadengine
LOCAL_SRC_FILES	:= threadengine-jni.cpp ../thread.cpp ../ring_buffer.cpp ../audio_ring_buffer.cpp

TARGET_PLATFORM := android-16

LOCAL_LDLIBS	+= -std=gnu++11 -llog 

LOCAL_CFLAGS	+= -Wall

LOCAL_C_INCLUDES += ../../include

include $(BUILD_SHARED_LIBRARY)