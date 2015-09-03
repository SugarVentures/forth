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
	protected:
		AudioDevice m_device;
	};
} // oppvs

#endif // OPPVS_AUDIO_CAPTURE_HPP
