#ifndef OPPVS_AUDIO_CAPTURE_HPP
#define OPPVS_AUDIO_CAPTURE_HPP

#include "audio_device.hpp"

namespace oppvs {
	class AudioCapture
	{
	public:
		AudioCapture() {}
		AudioCapture(const AudioDevice& device) : m_device(device) {}
		virtual ~AudioCapture() {}
		virtual int init() { return 0;}
		virtual int start() { return 0; }
		virtual int stop() { return 0; }

		audio_callback callbackAudio;
		void* user;
	protected:
		AudioDevice m_device;
		GenericAudioBufferList m_callbackBuffer;
	};
} // oppvs

#endif // OPPVS_AUDIO_CAPTURE_HPP
