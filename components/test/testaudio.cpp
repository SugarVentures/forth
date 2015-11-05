#include "datatypes.hpp"
#include "mac_audio_engine.hpp"
#include "mac_audio_play.hpp"

#include <signal.h>
#include <iostream>

#include "mac_audio_tool.hpp"
#include "audio_opus_encoder.hpp"
#include "audio_opus_decoder.hpp"

#include "audio_ring_buffer.h"
#include <thread>
#include <chrono>
#include <mutex>


#define BUFFER_LEN 4096

using namespace oppvs;

AudioFileID fOutputAudioFile;
bool interrupt;


CARingBuffer *mBuffer;
AudioBufferList *mAudioBufferList;
MacAudioPlay* pplayer;

AudioOpusEncoder *pencoder;
AudioOpusDecoder *pdecoder;
float *in;
uint8_t *out;
float *outDecode;
uint64_t totalPos;
uint32_t oldNoFrames;
double oldTime;

AudioRingBuffer* ringBuffer;

OSStatus configureOutputFile()
{
    OSStatus err = noErr;
    CAStreamBasicDescription format;
    format.mSampleRate = 48000;
    format.mFormatID = kAudioFormatLinearPCM;
    format.mBitsPerChannel = 32;
    format.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
    format.mFramesPerPacket = 1;
    format.mChannelsPerFrame = 2;
    format.mBytesPerFrame = sizeof(Float32) * format.mChannelsPerFrame;
    format.mBytesPerPacket = format.mFramesPerPacket * format.mBytesPerFrame;

    CFURLRef destinationURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
                                                            CFSTR("/Users/caominhtrang/Desktop/testaudio.wav"),
                                                            kCFURLPOSIXPathStyle,
                                                            false);
    
    err = AudioFileCreateWithURL(destinationURL, kAudioFileCAFType, &format, kAudioFileFlags_EraseFile, &fOutputAudioFile);
    
    return err;

}

void encode(void* p)
{
    printf("Encode process\n");
    if (in == NULL)
    {
        in = new float[6000];
        memset(in, 0, 6000 * 4);
    }

    while (!interrupt)
    {
        if (ringBuffer->getNumberFrames() > 0)
        {
            if (ringBuffer->getNumberFrames() > 960)
            {
                oldNoFrames = 960;
                int err = ringBuffer->fetch(oldNoFrames, in, ringBuffer->getStartTime());
                printf("Fetch error: %d read frames: %d\n", err, oldNoFrames);
                
                
                int inLen = oldNoFrames * 8;
                int outLen = pencoder->encode(in, inLen, 1, out);
                
                printf("Encode Out Len: %d\n", outLen);
                
                if (outDecode == NULL)
                {
                    outDecode = new float[4000];
                }
                int decodeLen = pdecoder->decode(out, outLen, 1, outDecode);
                delete [] out;
                printf("Decode Out Len: %d\n", decodeLen);
                
                if (decodeLen > 0)
                {
                    UInt32 noWriteBytes = decodeLen * 8;
                    
                    OSStatus error = AudioFileWritePackets(fOutputAudioFile, false, noWriteBytes, nil, totalPos, &oldNoFrames, outDecode);
                    if (!error)
                        totalPos += oldNoFrames;
                }
            }
            else
                oldNoFrames = ringBuffer->getNumberFrames();
            
            

            /*UInt32 noWriteBytes = oldNoFrames * 8;

            OSStatus error = AudioFileWritePackets(fOutputAudioFile, false, noWriteBytes, nil, totalPos, &oldNoFrames, outDecode);
            if (!error)
                totalPos += oldNoFrames;*/
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    
}



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
            interrupt = true;
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
	
	if (!pplayer)
		return;
	
    uint32_t noFrames = ab.nFrames;
    if (pplayer->getFirstInputTime() < 0.)
    {
        pplayer->setFirstInputTime(ab.sampleTime);
        totalPos = 0;
    }
    
    oppvs::RingBufferError err = ringBuffer->store(&noFrames, ab.buffers[0].data, ab.sampleTime);


	/*int len = pencoder->encode(in, 1, (float*)out);
	printf("Encoded len: %d\n", len);
	int len2 = pdecoder->decode(out, len, 1, outDecode);
	printf("Decoded len: %d\n", len2);*/


	
}

int main(int argc, char const *argv[])
{
	int error = 0;
    interrupt = false;

	signal(SIGPIPE, SIG_IGN);
    initAppExitListener();

    pplayer = NULL;
    //pencoder = NULL;
    in = NULL;
    out = NULL;

    mAudioBufferList = NULL;
    
    ringBuffer = new AudioRingBuffer();
    ringBuffer->allocate(8, 10 * 512);

	MacAudioEngine engine;
	std::vector<AudioDevice> devices;

	engine.callbackAudio = audioCallback;
	engine.getListAudioDevices(devices, false);
	engine.printAudioDeviceList();

	uint32_t deviceid = 40;
	if (engine.init() < 0)
		return -1;

	if (engine.addNewCapture(deviceid, NULL) < 0)
	{
		return -1;
	}
	printf("Successful to add new audio capture\n");

	
	AudioStreamInfo info;
	info.noSources = 1;
	info.sources = new AudioSourceInfo[info.noSources];
	info.sources[0].source = 1;
	info.sources[0].type = AUDIO_TYPE_MIXED;
	info.sources[0].format = AUDIO_FORMAT_FLOAT;
	info.sources[0].numberChannels = 2;
	info.sources[0].samplePerChannels = 512;
	info.sources[0].sampleRate = 48000;
    
    AudioOpusEncoder encoder;
	if (encoder.init(info) < 0)
        return -1;
    pencoder = &encoder;

	AudioOpusDecoder decoder;
	if (decoder.init(info) < 0)
        return -1;
	pdecoder = &decoder;

	AudioDevice output(207);
	MacAudioPlay player(output, 48000, 2);
	pplayer = &player;
	player.attachBuffer(ringBuffer);
	if (player.init() < 0)
        return -1;
    
    totalPos = 0;
    configureOutputFile();
	player.start();
    //void* p;
    //std::thread t1(encode, p);
    //t1.join();

	waitForAppExitSignal();
	player.stop();

	delete [] in;
	delete [] out;
    
    delete ringBuffer;
	printf("End programing\n");
	return 0;
}