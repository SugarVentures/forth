#include "datatypes.hpp"
#include "mac_audio_engine.hpp"
#include "mac_audio_play.hpp"
#include "mac_utility/CARingBuffer.h"

#include <signal.h>
#include <iostream>

#include "mac_audio_tool.hpp"
#include "audio_opus_encoder.hpp"
#include "audio_opus_decoder.hpp"

using namespace oppvs;
CARingBuffer *mBuffer;
AudioBufferList *mAudioBufferList;
MacAudioPlay* pplayer;

AudioOpusEncoder *pencoder;
AudioOpusDecoder *pdecoder;
float *in;
uint8_t *out;

int initAppExitListener()
{
    sigset_t sigs;   
    int ret;
    sigemptyset(&sigs);
    sigaddset(&sigs, SIGINT);
    sigaddset(&sigs, SIGTERM);

    ret = pthread_sigmask(SIG_BLOCK, &sigs, NULL);
    if (ret > 0)
    	std::cout << "exit error: " << ret << std::endl;
    return 0;
}

void waitForAppExitSignal()
{
    while (true)
    {
        sigset_t sigs;
        sigemptyset(&sigs);
        sigaddset(&sigs, SIGINT);
        sigaddset(&sigs, SIGTERM);
        int sig = 0;
        
        int ret = sigwait(&sigs, &sig);
        if ((sig == SIGINT) || (sig == SIGTERM))
        {
            break;
        }
    }
}

void audioCallback(GenericAudioBufferList& ab)
{
	/*printf("Call back audio\n");
	printf("Number of buffers: %d\n", ab.nBuffers);
	printf("Number of frames: %d\n", ab.nFrames);
	for (unsigned i = 0; i < ab.nBuffers; ++i) {
		printf("Data Size: %d Interleaved Channel: %d\n", ab.buffers[i].dataLength, ab.buffers[i].numberChannels);
	}*/
	
	if (!pplayer || !pencoder)
		return;
	if (in == NULL)
	{
		int totalsize = 0;
		for(unsigned i = 0; i < ab.nBuffers; ++i) {
			totalsize += ab.buffers[i].dataLength;
		}
		in = new float[totalsize / sizeof(float)];

	}
	int pos = 0;
	for(unsigned i = 0; i < ab.nBuffers; ++i) {
		memcpy(in + pos, ab.buffers[i].data, ab.buffers[i].dataLength);
		pos += ab.buffers[i].dataLength;
	}

	if (out == NULL)
	{
		out = new uint8_t[AUDIO_MAX_ENCODING_PACKET_SIZE];
	}
	int len = pencoder->encode(in, 1, out);

	if (pplayer->getFirstInputTime() < 0.)
		pplayer->setFirstInputTime(ab.sampleTime);
	convertGenericABLToABL(&ab, mAudioBufferList);
	if (mBuffer)
		mBuffer->Store(mAudioBufferList, ab.nFrames, ab.sampleTime);
}

int main(int argc, char const *argv[])
{
	signal(SIGPIPE, SIG_IGN);    
    initAppExitListener();

    pplayer = NULL;
    pencoder = NULL;
    in = NULL;
    out = NULL;

    mBuffer = NULL;
    mBuffer = new CARingBuffer();
    mBuffer->Allocate(2, 4, 512 * 20);

    mAudioBufferList = NULL;

	MacAudioEngine engine;
	std::vector<AudioDevice> devices;

	engine.callbackAudio = audioCallback;
	engine.getListAudioDevices(devices);
	engine.printAudioDeviceList();

	uint32_t deviceid = 96;
	if (engine.init() < 0)
		return -1;

	if (engine.addNewCapture(deviceid) < 0)
	{
		return -1;
	}
	printf("Successful to add new audio capture\n");

	AudioOpusEncoder encoder;
	AudioStreamInfo info;
	info.noSources = 1;
	info.sources = new AudioSourceInfo[info.noSources];
	info.sources[0].source = 1;
	info.sources[0].type = AUDIO_TYPE_MIXED;
	info.sources[0].format = AUDIO_FORMAT_FLOAT;
	info.sources[0].numberChannels = 2;
	info.sources[0].samplePerChannels = 512;
	info.sources[0].sampleRate = 44100;
	encoder.init(info);
	pencoder = &encoder;

	AudioOpusDecoder decoder;
	decoder.init(info);
	pdecoder = &decoder;

	AudioDevice output(40);
	MacAudioPlay player(output, 44100, 2);
	pplayer = &player;
	player.attachBuffer(mBuffer);
	player.init();
	player.start();

	waitForAppExitSignal();
	player.stop();

	delete [] in;
	delete [] out;
	printf("End programing\n");
	return 0;
}