#ifndef FORTH_AUDIO_DEVICE_H
#define FORTH_AUDIO_DEVICE_H

#include <IOKit/audio/IOAudioDevice.h>

#define AUDIO_ENGINES_KEY				"AudioEngines"
#define DESCRIPTION_KEY					"Description"
#define BLOCK_SIZE_KEY					"BlockSize"
#define NUM_BLOCKS_KEY					"NumBlocks"
#define NUM_STREAMS_KEY					"NumStreams"
#define FORMATS_KEY						"Formats"
#define SAMPLE_RATES_KEY				"SampleRates"

#define ForthAudioDevice com_Sugar_driver_ForthAudioDevice

class ForthAudioDevice: public IOAudioDevice
{
    
};
#endif
