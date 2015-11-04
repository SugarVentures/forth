#ifndef OPPVS_AUDIO_UTIL_H
#define OPPVS_AUDIO_UTIL_H

#ifdef FORTH_IOS
#import <CoreFoundation/CoreFoundation.h>

typedef UInt32 AudioDeviceID;
#define kAudioUnitSubType_HALOutput kAudioUnitType_Output;
#define kAudioUnitSubType_DefaultOutput kAudioUnitSubType_RemoteIO;
#endif


#endif


