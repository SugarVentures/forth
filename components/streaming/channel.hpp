/*
	Channel for communication	
*/

#ifndef OPPVS_CHANNEL_HPP
#define OPPVS_CHANNEL_HPP

#include "datatypes.hpp"
#include "network.hpp"

namespace oppvs
{

	class Channel
	{
	public:
		Channel();
		virtual ~Channel() {}

		virtual int init() { return 0; }		
	protected:
		SocketAddress m_localAddress;		
	};
}

#endif