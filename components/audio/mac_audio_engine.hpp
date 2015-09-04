#include "audio_engine.hpp"
#include "mac_audio_capture.hpp"

#import <AudioToolbox/AudioToolbox.h>
#import <CoreFoundation/CoreFoundation.h>

#include <vector>

namespace oppvs {

	class MacAudioEngine : public AudioEngine
	{
	public:
		MacAudioEngine();
		~MacAudioEngine();
		
		void getListAudioDevices(std::vector<AudioDevice>& result);
		int addNewCapture(uint32_t deviceid);
		int init();
		int shutdown();
	private:
		std::vector<MacAudioCapture*> m_listCaptures;
	};
} // oppvs