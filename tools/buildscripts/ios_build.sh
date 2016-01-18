#!/bin/sh

HERE=$(dirname $0)
ROOT_DIR=${HERE}/../..

#Go to components folders
OUTPUT_DIR="ios"
OS_VAR="ios"

cd "${ROOT_DIR}/components/audio"
gyp audio_engine.gyp --depth=. -f xcode -DOS=${OS_VAR} --generator-output=${OUTPUT_DIR}

cd "${ROOT_DIR}/components/misc"
gyp misc.gyp --depth=. -f xcode -DOS=${OS_VAR} --generator-output=${OUTPUT_DIR}

cd "${ROOT_DIR}/components/thread"
gyp thread_engine.gyp --depth=. -f xcode -DOS=${OS_VAR} --generator-output=${OUTPUT_DIR}

cd "${ROOT_DIR}/components/encoding"
gyp encoding_engine.gyp --depth=. -f xcode -DOS=${OS_VAR} --generator-output=${OUTPUT_DIR}

cd "${ROOT_DIR}/components/network"
gyp network_engine.gyp --depth=. -f xcode -DOS=${OS_VAR} --generator-output=${OUTPUT_DIR}

cd "${ROOT_DIR}/libs/libyuv"
gyp libyuv.gyp --depth=. -f xcode -DOS=${OS_VAR} --generator-output=${OUTPUT_DIR}

cd "${ROOT_DIR}/components/streaming"
gyp streaming_engine.gyp --depth=. -f xcode -DOS=${OS_VAR} --generator-output=${OUTPUT_DIR}
