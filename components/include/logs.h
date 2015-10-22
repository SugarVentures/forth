#ifndef OPPVS_LOGS_H
#define OPPVS_LOGS_H

#ifdef ANDROID

#define  LOG_TAG    "OPPVS"

#include <android/log.h>

#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#else

#define LOGD(...) printf(__VA_ARGS__);
#define LOGE(...) printf(__VA_ARGS__);

#endif


#endif // OPPVS_LOGS_H
