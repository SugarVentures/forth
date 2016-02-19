#include "forth.h"

#include "logs.h"
#include "ice_manager.hpp"

namespace oppvs
{
	void test()
	{
		std::string address("192.168.1.9");
		IceServerInfo stunServer(address, DEFAULT_STUN_PORT, "", "");
		IceServerInfo turnServer(address, DEFAULT_STUN_PORT, "turn", "password");

		oppvs::IceManager* icemgr = new oppvs::IceManager();
		int ret = icemgr->init(stunServer, turnServer, 0);
		LOGD("Ret: %d\n", ret);
 
	}
} //oppvs

