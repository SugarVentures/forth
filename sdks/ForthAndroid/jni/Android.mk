LOCAL_PATH := $(call my-dir)

define all-cpp-files-indir
$(patsubst ./%,%, \
	$(shell cd $(LOCAL_PATH) ; \
		find $(1) -name "*.cpp" -and -not -name ".*") \
)
endef

define all-cpp-files-inThread
$(call all-cpp-files-indir,../../../components/thread)
endef

include $(CLEAR_VARS)

LOCAL_MODULE	:= forthsdk
LOCAL_SRC_FILES	:= forth-jni.cpp
LOCAL_SRC_FILES += $(call all-cpp-files-inThread)

TARGET_PLATFORM := android-16

LOCAL_LDLIBS	+= -std=gnu++11 -llog 

LOCAL_CFLAGS	+= -Wall

LOCAL_C_INCLUDES += ../../../components/include
LOCAL_C_INCLUDES += ../../../components/thread

include $(BUILD_SHARED_LIBRARY)