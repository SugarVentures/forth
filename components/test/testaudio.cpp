#include "datatypes.hpp"
#include "mac_audio_engine.hpp"
#include "mac_audio_play.hpp"
#include "mac_utility/CARingBuffer.h"

#include <signal.h>
#include <iostream>

#include "mac_audio_tool.hpp"


using namespace oppvs;
CARingBuffer *mBuffer;
AudioBufferList *mAudioBufferList;
MacAudioPlay* pplayer;

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
	//if (mFirstInputTime < 0.)
	//	mFirstInputTime = ab.sampleTime;

	if (!pplayer)
		return;
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

	AudioDevice output(40);
	MacAudioPlay player(output, 44100, 2);
	pplayer = &player;
	player.attachBuffer(mBuffer);
	player.init();
	player.start();

	waitForAppExitSignal();
	player.stop();
	printf("End programing\n");
	return 0;
}