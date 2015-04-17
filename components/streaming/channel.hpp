/*
	Channel for communication	
*/

#ifndef OPPVS_CHANNEL_HPP
#define OPPVS_CHANNEL_HPP

#include "datatypes.hpp"
#include "network.hpp"

namespace oppvs
{
	enum ServiceType
	{
		ST_VIDEO_STREAMING
	};

	struct ServiceInfo
	{
		uint8_t type;
		uint32_t key;	//In case of video streaming, key = ssrc
	};


	class Channel
	{
	public:
		Channel();
		virtual ~Channel() {}

		virtual int init() { return 0; }
		const ServiceInfo& getServiceInfo();
		void setServiceInfo(uint8_t type, uint32_t key);
		void setServiceInfo(const ServiceInfo& s);
		const uint32_t getServiceKey();


	protected:
		SocketAddress m_localAddress;
		ServiceInfo m_service;	
	};
}

#endif