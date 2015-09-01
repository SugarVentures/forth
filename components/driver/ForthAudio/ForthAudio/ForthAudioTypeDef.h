//
//  ForthAudioTypeDef.h
//  ForthAudio
//
//  Created by Cao Minh Trang on 9/1/15.
//  Copyright (c) 2015 Cao Minh Trang. All rights reserved.
//

#ifndef ForthAudio_ForthAudioTypeDef_h
#define ForthAudio_ForthAudioTypeDef_h

#include <libkern/OSTypes.h>

#define INITIAL_SAMPLE_RATE 44100
#define INITIAL_IOBUFFER_FRAME_SIZE 16384

//	IORegistry keys that have the basic info about the driver
#define kForthAudio_RegistryKey_SampleRate			"sample rate"
#define kForthAudio_RegistryKey_RingBufferFrameSize	"buffer frame size"
#define kForthAudio_RegistryKey_DeviceUID			"device UID"

//	memory types
enum
{
    kForthAudio_Buffer_Status,
    kForthAudio_Buffer_Input,
    kForthAudio_Buffer_Output
};

//	user client method selectors
enum
{
    kForthAudio_Method_Open,				//	No arguments
    kForthAudio_Method_Close,			//	No arguments
    kForthAudio_Method_StartHardware,	//	No arguments
    kForthAudio_Method_StopHardware,		//	No arguments
    kForthAudio_Method_SetSampleRate,	//	One input: the new sample rate as a 64 bit integer
    kForthAudio_Method_GetControlValue,	//	One input: the control ID, One output: the control value
    kForthAudio_Method_SetControlValue,	//	Two inputs, the control ID and the new value
    kForthAudio_Method_NumberOfMethods
};

//	control IDs
enum
{
    kForthAudio_Control_MasterInputVolume,
    kForthAudio_Control_MasterOutputVolume
};

//	volume control ranges
#define kForthAudio_Control_MinRawVolumeValue	0
#define kForthAudio_Control_MaxRawVolumeValue	96
#define kForthAudio_Control_MinDBVolumeValue		-96.0f
#define kForthAudio_Control_MaxDbVolumeValue		0.0f

//	the struct in the status buffer
struct ForthAudioStatus
{
    volatile UInt64	mSampleTime;
    volatile UInt64	mHostTime;
};
typedef struct ForthAudioStatus	ForthAudioStatus;



#endif
