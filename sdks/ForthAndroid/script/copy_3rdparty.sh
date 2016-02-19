#bin/sh

SRC_PATH=~/cerbero/dist/android_armv7/lib
DST_PATH=../3rdparty

declare -a libarr=("glib-2.0" "gio-2.0" "gobject-2.0" "iconv" "gmodule-2.0" "intl" "ffi" "z" "crypto" "gthread-2.0" "nice" "opus" "vpx" "srtp" "crypto" "ssl")

for libname in "${libarr[@]}"
do
	cp ${SRC_PATH}/lib${libname}.a ${DST_PATH}/	
done

