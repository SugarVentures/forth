#include "datatypes.hpp"
#include "mac_audio_engine.hpp"
#include "mac_audio_play.hpp"
#include "mac_utility/CARingBuffer.h"

#include <signal.h>
#include <iostream>

#include "mac_audio_tool.hpp"
#include "audio_opus_encoder.hpp"
#include "audio_opus_decoder.hpp"


#include "samplerate.h"

#define BUFFER_LEN 4096
#define DEFAULT_CONVERTER SRC_SINC_BEST_QUALITY

using namespace oppvs;
CARingBuffer *mBuffer;
AudioBufferList *mAudioBufferList;
MacAudioPlay* pplayer;

AudioOpusEncoder *pencoder;
AudioOpusDecoder *pdecoder;
float *in;
int *out;
float *outDecode;

SRC_STATE* src_state;
SRC_DATA src_data;

SRC_STATE* dest_state;
SRC_DATA dest_data;



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
		uint8_t* data = (uint8_t*)ab.buffers[i].data;
		printf("Data Size: %d \n", ab.buffers[i].dataLength);
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
		//memcpy(in + pos, ab.buffers[i].data, ab.buffers[i].dataLength);
		pos += ab.buffers[i].dataLength;
	}

	/*if (out == NULL)
	{
		out = new int[AUDIO_MAX_ENCODING_PACKET_SIZE];
		outDecode = new float[2*2048];
		memset(outDecode, 0, 2*2048);
	}

	int outLen = 0;
	int channel[2];
	
	src_data.data_in = in;
	src_data.input_frames = 1024;
	src_data.data_out = outDecode;

	int error = src_process(src_state, &src_data);
	if (error)
	{
		printf ("\nError : %s\n", src_strerror (error));
	}
	printf("out len of resampler: %d used frames: %d\n", src_data.output_frames_gen, src_data.input_frames_used);
	outLen = src_data.output_frames_gen;

	memset(in, 0, 4096);
	dest_data.data_in = outDecode;
	dest_data.input_frames = outLen;
	dest_data.data_out = in;

	error = src_process(dest_state, &dest_data);
	if (error)
	{
		printf ("\nError : %s\n", src_strerror (error));
	}
	printf("out len of resampler (rev): %d\n", dest_data.output_frames_gen);
	outLen = dest_data.output_frames_gen;
	

	/*int len = pencoder->encode(in, 1, (float*)out);
	printf("Encoded len: %d\n", len);
	int len2 = pdecoder->decode(out, len, 1, outDecode);
	printf("Decoded len: %d\n", len2);*/

	pos = 0;
	for(unsigned i = 0; i < ab.nBuffers; ++i) {
		//memcpy(ab.buffers[i].data, in + pos, ab.buffers[i].dataLength);
		pos += ab.buffers[i].dataLength;
	}	

	if (pplayer->getFirstInputTime() < 0.)
		pplayer->setFirstInputTime(ab.sampleTime);

	convertGenericABLToABL(&ab, mAudioBufferList);
	printf("nFrames: %d\n", ab.nFrames);
	//ab.nFrames = 512;
	if (mBuffer)
	{
		/*printf("audiobuffer list %d\n", mAudioBufferList->mNumberBuffers);
			uint8_t* data = (uint8_t*)mAudioBufferList->mBuffers[0].mData;
			for (int i = 0; i < mAudioBufferList->mBuffers[0].mDataByteSize; i++)
			{
				printf("%d", data[i]);
			}
			printf("\n");*/

		int err = mBuffer->Store(mAudioBufferList, ab.nFrames, ab.sampleTime);
		//printf("Err: %d\n", err);
	}
}

int main(int argc, char const *argv[])
{
	int error = 0;
	src_state = src_delete(src_state);
	dest_state = src_delete(dest_state);

	if ((src_state = src_new(DEFAULT_CONVERTER, 2, &error)) == NULL)
	{
		printf ("\n\nError : src_new() failed : %s.\n\n", src_strerror (error));
	}
	src_data.end_of_input = 0;

	/* Start with zero to force load in while loop. */
	src_data.input_frames = 0 ;
	src_data.src_ratio = (double)48000/(double)44100;
	src_data.output_frames = BUFFER_LEN / 2;

	if ((dest_state = src_new(DEFAULT_CONVERTER, 2, &error)) == NULL)
	{
		printf ("\n\nError : src_new() failed : %s.\n\n", src_strerror (error));
	}
	dest_data.end_of_input = 0;

	/* Start with zero to force load in while loop. */
	dest_data.input_frames = 0 ;
	dest_data.src_ratio = (double)44100/(double)48000;
	dest_data.output_frames = BUFFER_LEN / 2;


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
	MacAudioPlay player(output, 48000, 2);
	pplayer = &player;
	player.attachBuffer(mBuffer);
	player.init();
	player.start();

	waitForAppExitSignal();
	player.stop();

	src_state = src_delete(src_state);
	dest_state = src_delete(dest_state);
	delete [] in;
	delete [] out;
	printf("End programing\n");
	return 0;
}