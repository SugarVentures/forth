
#include "../streaming/streaming_engine.hpp"

void frameCallback(oppvs::PixelBuffer& pf)
{
	printf("Out data: %d %d\n", pf.width[0], pf.plane[0][20]);
}

int main(int argc, char* argv[])
{
	
	oppvs::PixelBuffer pf;
	pf.width[0] = 1280;
	pf.height[0] = 780;
	pf.stride[0] = 10;
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
	    while (1)
	    {	
	    	se->pushData(pf);
	    	usleep(30000);
	    }
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
		while (1)
		{
			usleep(100);
		}
	}
	return 0;
}