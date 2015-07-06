#include "stun_server.hpp"

namespace oppvs
{
	StunServer::StunServer() : mSockets()
	{

	}

	StunServer::~StunServer()
	{
		shutdown();
	}

	int StunServer::init()
	{
		return 0;
	}

	int StunServer::shutdown()
	{
		return 0;
	}

	int StunServer::start()
	{
		return 0;
	}

	int StunServer::stop()
	{
		return 0;
	}
}