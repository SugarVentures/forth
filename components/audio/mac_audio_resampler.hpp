#ifndef OPPVS_MAC_AUDIO_RESAMPLER_HPP
#define OPPVS_MAC_AUDIO_RESAMPLER_HPP

#import <AudioToolbox/AudioToolbox.h>
#import <CoreFoundation/CoreFoundation.h>
#include "mac_utility/CAStreamBasicDescription.h"

namespace oppvs {
	class MacAudioResampler
	{
	public:
		MacAudioResampler();
		~MacAudioResampler();
		
		int init(CAStreamBasicDescription& inputFormat, CAStreamBasicDescription& outputFormat);
		void cleanup();
		int convert(AudioConverterComplexInputDataProc proc, void* userData, UInt32 *ioOutputDataPackets, AudioBufferList* abl);
		AudioBufferList* getBuffer();
	private:
		AudioConverterRef m_converter;
		AudioBufferList* m_buffer;
		UInt32 m_ioOutputDataPackets;
	};
} // oppvs
	
#endif // OPPVS_MAC_AUDIO_RESAMPLER_HPP
