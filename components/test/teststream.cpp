
#include "../streaming/streaming_engine.hpp"

int interrupt;

void frameCallback(oppvs::PixelBuffer& pf)
{
	printf("Out data: %d %d %d %d %d %d\n", pf.width[0], pf.height[0], pf.originx, pf.originy, pf.plane[0][100], pf.plane[0][200]);
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
	pf.width[0] = 400;
	pf.height[0] = 300;
	pf.stride[0] = pf.width[0]*4;
	pf.nbytes = pf.height[0] * pf.stride[0];
	pf.plane[0] = new uint8_t[pf.nbytes];
	memset(pf.plane[0], 1, pf.nbytes);


	oppvs::PixelBuffer rcvpf;

	int port = 0;
	uint32_t ssrc = 0;
	if (argc > 1)
	{
		port = atoi(argv[1]);

	}
	if (port == 0)
	{
		oppvs::StreamingEngine *se = new oppvs::StreamingEngine();
		se->setup(&pf);
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
		oppvs::StreamingEngine *se = new oppvs::StreamingEngine();
		se->setup(&rcvpf);
		se->registerCallback(frameCallback);
		oppvs::ServiceInfo service;
		service.type = oppvs::ST_VIDEO_STREAMING;
		service.key = ssrc;
		se->initSubscribeChannel("127.0.0.1", port, service);
		
		while (!interrupt)
		{
			usleep(5000);
		
		}
		//delete se;
	}

	return 0;
}

