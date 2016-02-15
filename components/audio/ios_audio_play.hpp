#ifndef OPPVS_IOS_AUDIO_PLAY_HPP
#define OPPVS_IOS_AUDIO_PLAY_HPP

#include "mac_audio_play.hpp"

namespace oppvs {
	class IosAudioPlay : public MacAudioPlay {
	public:
		IosAudioPlay(const AudioDevice& device, uint64_t isr, uint32_t inc) : MacAudioPlay(device, isr, inc) {}
	};
} // oppvs

#endif // OPPVS_IOS_AUDIO_PLAY_HPP
