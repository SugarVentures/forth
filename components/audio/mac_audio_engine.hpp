#include "audio_engine.hpp"

namespace oppvs {
	class MacAudioEngine : public AudioEngine
	{
	public:
		MacAudioEngine();
		~MacAudioEngine();
		
		void getListAudioDevices(std::vector<AudioDevice>& result);
	};
} // oppvs