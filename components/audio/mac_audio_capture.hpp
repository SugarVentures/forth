#ifndef OPPVS_MAC_AUDIO_CAPTURE_HPP
#define OPPVS_MAC_AUDIO_CAPTURE_HPP

#include "audio_capture.hpp"

#import <AudioToolbox/AudioToolbox.h>
#import <CoreFoundation/CoreFoundation.h>

namespace oppvs {
	class MacAudioCapture : public AudioCapture
	{
	public:
		MacAudioCapture(const AudioDevice& device) : AudioCapture(device) {}
		int init();
	private:
		AudioComponentInstance m_auHAL;

		int createAudioOutputUnit();
		void enableIO();
		int setInputDevice(AudioDeviceID deviceid);
		void setupCallback();

		static OSStatus AudioInputProc(void* inRefCon, AudioUnitRenderActionFlags* ioActionFlags, const AudioTimeStamp* inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList* ioData);
	};
} // oppvs

#endif // OPPVS_MAC_AUDIO_CAPTURE_HPP
