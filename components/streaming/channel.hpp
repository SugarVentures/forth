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

	struct ChannelMessage
	{
		uint16_t width;
		uint16_t height;
		uint16_t stride;
		uint8_t flip;
		sockaddr destination;
	};


	class Channel
	{
	public:
		Channel();
		virtual ~Channel() {}

		virtual int init() { return 0; }
		const ServiceInfo& getServiceInfo() const;
		void setServiceInfo(uint8_t type, uint32_t key);
		void setServiceInfo(const ServiceInfo& s);
		uint32_t getServiceKey() const;
		const SocketAddress& getLocalAddress() const;

	protected:
		SocketAddress m_localAddress;
		ServiceInfo m_service;	
	};
}

#endif