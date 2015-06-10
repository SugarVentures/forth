/*
	Channel for communication	
*/

#ifndef OPPVS_CHANNEL_HPP
#define OPPVS_CHANNEL_HPP

#include "datatypes.hpp"
#include "network.hpp"
#include "thread.hpp"

namespace oppvs
{
	enum ServiceType
	{
		ST_VIDEO_STREAMING
	};



	struct ChannelMessage
	{
		uint16_t videoWidth;
		uint16_t videoHeight;
		sockaddr destination;
		uint8_t noSource;
	};

	
	class Channel
	{
	public:
		Channel();
		virtual ~Channel();

		virtual int init() { return 0; }
		const ServiceInfo& getServiceInfo() const;
		void setServiceInfo(uint8_t type, uint32_t key);
		void setServiceInfo(const ServiceInfo& s);
		void setServiceInfo(ServiceInfo *s);
		uint32_t getServiceKey() const;
		const SocketAddress& getLocalAddress() const;

		uint16_t getMessageSize();
		uint8_t* createMessage(sockaddr* destination);
		void parseMessage(uint8_t* msg, int len, sockaddr* destination);
		void releaseMessage();
	protected:
		SocketAddress m_localAddress;
		ServiceInfo m_service;
		ServiceInfo *p_service;
		uint8_t *m_message;
	};
}

#endif