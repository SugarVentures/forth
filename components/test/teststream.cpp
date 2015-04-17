
#include "../streaming/streaming_engine.hpp"

int main(int argc, char* argv[])
{
	oppvs::StreamingEngine se;
	int port = 0;
	uint32_t ssrc = 0;
	if (argc > 1)
	{
		port = atoi(argv[1]);
		ssrc = atol(argv[2]);
	}
	if (port == 0)
	{
	    se.initPublishChannel();
	    while (1)
	    {
	    	usleep(100);
	    }
	}
	else
	{
		oppvs::ServiceInfo service;
		service.type = oppvs::ST_VIDEO_STREAMING;
		service.key = ssrc;
		se.initSubscribeChannel("127.0.0.1", port, service);
		while (1)
		{
			usleep(100);
		}
	}
	return 0;
}