#include "audio_engine.hpp"

#import <AudioToolbox/AudioToolbox.h>
#import <CoreFoundation/CoreFoundation.h>

namespace oppvs {
	class MacAudioEngine : public AudioEngine
	{
	public:
		MacAudioEngine();
		~MacAudioEngine();
		
		void getListAudioDevices(std::vector<AudioDevice>& result);
	};
} // oppvs