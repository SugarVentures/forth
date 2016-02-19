#bin/sh

HERE=$(dirname $0)
cd ${HERE}/../jni

ndk-build

LIB_PATH="${HERE}/../obj/local/*"

if [ "$#" -gt 0 ]
then
	#Copy lib files
	mkdir -p $1/libs
	cp -r ${LIB_PATH} $1/libs

	#Copy template .mk
	mkdir -p $1/jni
	cp ${HERE}/../jni/common.mk $1/jni

	#Copy header
	cp ${HERE}/../jni/*.h $1/libs

	#Copy third party libs
	cp -r ${HERE}/../3rdparty $1
fi


