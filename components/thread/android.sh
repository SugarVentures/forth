#!/bin/sh
# 

#################### GYP_DEFINES ####
DEFINES=" library=static_library
          os_posix=1
          OS=android
          target_arch=arm
          clang=0
          component=static_library
          use_system_stlport=0
        "

HERE=$(dirname $0)
ROOT_DIR=${HERE}/../..

# Off we go.
ANDROID_BUILD_TOP=`pwd` GYP_DEFINES=${DEFINES} GYP_GENERATORS=make-android ${ROOT_DIR}/components/thread/gyp_build --depth=. --generator-output=build *.gyp $*

