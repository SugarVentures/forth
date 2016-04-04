LOCAL_PATH := $(call my-dir)

include $(LOCAL_PATH)/../sdk/jni/common.mk

include $(CLEAR_VARS)
LOCAL_MODULE := forthsdk
LOCAL_SRC_FILES := ../sdk/libs/$(TARGET_ARCH_ABI)/libforthsdk.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE	:= forthwrapper
LOCAL_SRC_FILES	:= forth-jni.cpp

TARGET_PLATFORM := android-16

LOCAL_LDLIBS	+= -std=gnu++11 -llog -pthread -lEGL -lGLESv2

LOCAL_CFLAGS	+= -Wall

LOCAL_STATIC_LIBRARIES := forthsdk nice glib-2.0 gio-2.0 gobject-2.0 glib-2.0 gmodule-2.0 glib-2.0 intl ffi z iconv vpx opus

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../sdk/libs

include $(BUILD_SHARED_LIBRARY)