LOCAL_PATH := $(call my-dir)

define all-cpp-files-indir
$(patsubst ./%,%, \
	$(shell cd $(LOCAL_PATH) ; \
		find $(1) -name "*.cpp" -o -name "*.c" -and -not -name ".*") \
)
endef

define all-cpp-files-inThread
$(call all-cpp-files-indir,../../../components/thread)
endef

define all-cpp-files-inMisc
$(call all-cpp-files-indir,../../../components/misc)
endef

define all-cpp-files-inNetwork
$(call all-cpp-files-indir,../../../components/network)
endef


include common.mk


include $(CLEAR_VARS)
LOCAL_MODULE	:= forthsdk
LOCAL_SRC_FILES	:= forth-jni.cpp
LOCAL_SRC_FILES += $(call all-cpp-files-inThread)
LOCAL_SRC_FILES += $(call all-cpp-files-inMisc)
LOCAL_SRC_FILES += $(call all-cpp-files-inNetwork)

TARGET_PLATFORM := android-16

#LOCAL_LDLIBS	+= -std=gnu++11 -llog -pthread

LOCAL_CFLAGS	+= -Wall

LOCAL_C_INCLUDES += ../../../components/include
LOCAL_C_INCLUDES += ../../../components/thread
LOCAL_C_INCLUDES += ../../../components/misc
LOCAL_C_INCLUDES += ../../../components/network
LOCAL_C_INCLUDES += /Users/caominhtrang/cerbero/dist/android_armv7/include
LOCAL_C_INCLUDES += /Users/caominhtrang/cerbero/dist/android_armv7/include/glib-2.0
LOCAL_C_INCLUDES += /Users/caominhtrang/cerbero/dist/android_armv7/lib/glib-2.0/include

LOCAL_STATIC_LIBRARIES := iconv glib-2.0 ffi intl gio-2.0 gobject-2.0 gmodule-2.0 gthread-2.0 z nice srtp crypto

include $(BUILD_STATIC_LIBRARY)