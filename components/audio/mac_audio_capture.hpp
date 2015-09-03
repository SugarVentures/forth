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

		int createAudioOutputUnit(AudioComponentInstance* pinstance);
		void enableIO(AudioComponentInstance& instance);
	};
} // oppvs

#endif // OPPVS_MAC_AUDIO_CAPTURE_HPP
