
#include <iostream>
#include <time.h>
#include <chrono>

#include "streaming_engine.hpp"

void frameCallback(oppvs::PixelBuffer& pf)
{
	printf("callback\n");
}

void streamingCallback(void* user)
{
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
        printf("quit %d\n", ret);
        if ((sig == SIGINT) || (sig == SIGTERM))
        {
        	
            break;
        }
    }
}

int main(int argc, char* argv[])
{
	signal(SIGPIPE, SIG_IGN);
    
    initAppExitListener();
    
	oppvs::StreamingEngine *streamEngine;	
	oppvs::AudioRingBuffer mAudioRingBuffer;

	streamEngine = new oppvs::StreamingEngine();
    streamEngine->registerCallback(frameCallback);
    streamEngine->attachBuffer(&mAudioRingBuffer);
    streamEngine->registerCallback(streamingCallback, NULL);

    if (streamEngine->init(oppvs::ROLE_VIEWER, oppvs::STUN_SERVER_ADDRESS, oppvs::TURN_SERVER_ADDRESS, oppvs::TURN_SERVER_USER, oppvs::TURN_SERVER_PASS,
                               oppvs::SIGN_SERVER_ADDRESS, oppvs::SIGN_SERVER_PORT) < 0)
    {
        printf("Failed to init streaming engine");
        return -1;
    }
    
    std::string strStreamKey("7116f0d7-5c27-44e6-8aa4-bc4ddeea9935");
    if (streamEngine->start(strStreamKey) < 0)
    {
        printf("Failed to start streaming engine");
        return -1;
    }

    waitForAppExitSignal();
	return 0;
}

