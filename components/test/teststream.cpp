
#include "../streaming/streaming_engine.hpp"

int interrupt;

void frameCallback(oppvs::PixelBuffer& pf)
{
	printf("Out data: %d %d %d %d\n", pf.width[0], pf.height[0], pf.originx, pf.originy);
}

void signalhandler(int param)
{
 	interrupt = 1;
}



int main(int argc, char* argv[])
{
	interrupt = 0;
	signal(SIGINT, signalhandler);

	oppvs::PixelBuffer pf;
	pf.width[0] = 1280;
	pf.height[0] = 780;
	pf.stride[0] = 120;
	pf.nbytes = pf.height[0] * pf.stride[0];
	pf.plane[0] = new uint8_t[pf.nbytes];
	memset(pf.plane[0], 1, pf.nbytes);


	oppvs::PixelBuffer rcvpf;

	int port = 0;
	uint32_t ssrc = 0;
	if (argc > 2)
	{
		port = atoi(argv[1]);
		ssrc = atol(argv[2]);

	}
	if (port == 0)
	{
		oppvs::StreamingEngine *se = new oppvs::StreamingEngine(&pf);

	    se->initPublishChannel();
	    while (!interrupt)
	    {	
	    	se->pushData(pf);
	    	usleep(300000);
	    }
	    delete se;
	}
	else
	{
		oppvs::StreamingEngine *se = new oppvs::StreamingEngine(&rcvpf);
		se->registerCallback(frameCallback);
		oppvs::ServiceInfo service;
		service.type = oppvs::ST_VIDEO_STREAMING;
		service.key = ssrc;
		se->initSubscribeChannel("127.0.0.1", port, service);
		printf("Stream info: %u %u %u\n", rcvpf.width[0], rcvpf.height[0], rcvpf.nbytes);
		while (!interrupt)
		{
			usleep(5000);
		
		}
		delete se;
	}

	return 0;
}

