#ifndef OPPVS_MAC_AUDIO_CAPTURE_HPP
#define OPPVS_MAC_AUDIO_CAPTURE_HPP

#include "audio_capture.hpp"

#import <AudioToolbox/AudioToolbox.h>
#import <CoreFoundation/CoreFoundation.h>
#include "mac_utility/CAStreamBasicDescription.h"


namespace oppvs {
	class MacAudioCapture : public AudioCapture
	{
	public:
		MacAudioCapture(const AudioDevice& device) : AudioCapture(device), m_audioBuffer(NULL) {}
		~MacAudioCapture();
		int init();
		int start();
		int stop();
	private:
		AudioComponentInstance m_auHAL;
		AudioBufferList* m_audioBuffer;
		CAStreamBasicDescription m_outputFormat;

		int createAudioOutputUnit();
		void enableIO();
		int setInputDevice(AudioDeviceID deviceid);
		void setupCallback();
		int setupBuffer();

		AudioBufferList* allocateAudioBufferListWithNumChannels(UInt32 numChannels, UInt32 size);
		void destroyAudioBufferList(AudioBufferList* list);

		static OSStatus AudioInputProc(void* inRefCon, AudioUnitRenderActionFlags* ioActionFlags, const AudioTimeStamp* inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList* ioData);
	};
} // oppvs

#endif // OPPVS_MAC_AUDIO_CAPTURE_HPP
