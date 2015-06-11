#include "channel.hpp"

namespace oppvs
{
	Channel::Channel()
	{
		IPAddress ipAddr;
		m_localAddress.setIP(ipAddr);
		m_localAddress.setPort(0);
		m_message = NULL;
		p_service = NULL;
	}

	Channel::~Channel()
	{
		if (m_message)
			releaseMessage();
	}

	const ServiceInfo& Channel::getServiceInfo() const {
		return m_service;
	}
	
	void Channel::setServiceInfo(uint8_t type, uint32_t key)
	{
		m_service.type = type;
		m_service.key = key;
	}

	void Channel::setServiceInfo(const ServiceInfo& s)
	{
		memcpy(&m_service, &s, sizeof(s));
	}

	void Channel::setServiceInfo(ServiceInfo *s)
	{
		p_service = s;
	}	

	uint32_t Channel::getServiceKey() const
	{
		return m_service.key;
	}

	const SocketAddress& Channel::getLocalAddress() const
	{
		return m_localAddress;
	}

	uint16_t Channel::getMessageSize()
	{
		uint16_t totalSize =  sizeof(sockaddr)	//Address of source of data
							+ sizeof(uint16_t)	//Video Width
							+ sizeof(uint16_t)	//Video Height
							+ sizeof(uint8_t);	//Number of Sources

		VideoSourceInfo info;
		totalSize += p_service->videoStreamInfo.noSources * info.size();
		return totalSize;
	}

	uint8_t* Channel::createMessage(sockaddr* destination)
	{
		if (!p_service)
		{
			return NULL;
		}

		if (m_message)
		{
			releaseMessage();
		}

		int curPos = 0;
		m_message = new uint8_t[getMessageSize()];
		memcpy(m_message + curPos, destination, sizeof(sockaddr));
		curPos += sizeof(sockaddr);
		memcpy(m_message + curPos, &p_service->videoStreamInfo.videoWidth, sizeof(uint16_t));
		curPos += sizeof(uint16_t);
		memcpy(m_message + curPos, &p_service->videoStreamInfo.videoHeight, sizeof(uint16_t));
		curPos += sizeof(uint16_t);
		memcpy(m_message + curPos, &p_service->videoStreamInfo.noSources, sizeof(uint8_t));
		curPos += sizeof(uint8_t);
		for (int i = 0; i < p_service->videoStreamInfo.noSources; i++)
		{
			memcpy(m_message + curPos, &p_service->videoStreamInfo.sources[i].source, sizeof(uint8_t));
			curPos += sizeof(uint8_t);
			memcpy(m_message + curPos, &p_service->videoStreamInfo.sources[i].order, sizeof(uint8_t));
			curPos += sizeof(uint8_t);
			memcpy(m_message + curPos, &p_service->videoStreamInfo.sources[i].width, sizeof(uint16_t));
			curPos += sizeof(uint16_t);
			memcpy(m_message + curPos, &p_service->videoStreamInfo.sources[i].height, sizeof(uint16_t));
			curPos += sizeof(uint16_t);
			memcpy(m_message + curPos, &p_service->videoStreamInfo.sources[i].stride, sizeof(uint16_t));
			curPos += sizeof(uint16_t);
		}
		return m_message;
	}

	void Channel::parseMessage(uint8_t* msg, int len, sockaddr* destination)
	{

	}

	void Channel::releaseMessage()
	{
		delete [] m_message;
		m_message = NULL;
	}
}