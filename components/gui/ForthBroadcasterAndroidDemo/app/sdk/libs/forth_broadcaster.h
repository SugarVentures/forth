#ifndef FORTH_BROADCASTER_H
#define FORTH_BROADCASTER_H

#include <jni.h>

namespace oppvs {
	class ForthBroadcaster
	{
	public:
		ForthBroadcaster();
		~ForthBroadcaster();

		static void Initialize(JNIEnv* env);
	};
} // oppvs

#endif // FORTH_BROADCASTER_H
