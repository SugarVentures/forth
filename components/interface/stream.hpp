/*
	Streaming Interface
*/

#ifndef OPPVS_STREAM_HPP
#define OPPVS_STREAM_HPP

#include <stdio.h>
#include <queue>

#include "../include/datatypes.hpp"
#include "../interface/network.hpp"
#include "../thread/concurrent_queue.hpp"

#define OPPVS_DEFAULT_SERVER_LISTEN_PORT 33432
#define OPPVS_IDLE_TIME	500
#define OPPVS_MAX_SENDING_QUEUE_LENGTH 10

namespace oppvs 
{
	struct StreamSetting
	{
		uint16_t port;
		uint16_t frameWidth;
		uint16_t frameHeight;
		uint16_t frameStride;

		StreamSetting() : port(OPPVS_DEFAULT_SERVER_LISTEN_PORT) {}		
	};

	class Frame {
	public:
		Frame();		
		~Frame();

		Frame(const PixelBuffer& pf, uint32_t ts);
		uint8_t* getData() const { return m_data;}
		uint32_t getLength() const { return m_length; }
		uint32_t getTimeStamp() const { return m_timestamp; }
		void setTimeStamp(uint32_t value) { m_timestamp = value; }

		Frame& operator=(const Frame& f) {
			if (&f == this)
				return *this;

			if (m_data != NULL)
			{
				if (m_length == f.getLength())
					memcpy(m_data, f.getData(), m_length);
				else
				{
					delete [] m_data;
					m_data = new uint8_t[f.getLength()];
					memcpy(m_data, f.getData(), f.getLength());
				}
			}
			else
			{
				m_data = new uint8_t[f.getLength()];
				memcpy(m_data, f.getData(), f.getLength());
			}
			m_length = f.getLength();
			m_timestamp = f.getTimeStamp();						
			return *this;
		}
	private:
		uint8_t* m_data;
		uint32_t m_length;
		uint32_t m_timestamp;
	};

	//Store information of each stream: source, encoding type, subscribers
	class Stream
	{
	public:
		Stream();
		Stream(const StreamSetting& setting);

		virtual ~Stream();
		int pushData(const PixelBuffer& pixelbuf);

		int initServer();
		ServerSocket* getServer();
		int initStreamingSocket(const SocketAddress&);
		void setIsStreaming(bool value);

		ConQueue<Frame*>* getQueue() { return &m_sendingQueue; }
		SRTPSocket* getStreamingSocket() { return &m_srtpSocket; }
		void setting(const StreamSetting& st) { m_setting = st;}

		const bool isInterrupt() { return interrupt; }

	private:
		ConQueue<Frame*> m_sendingQueue;
		uint16_t m_maxSendingQueueLength;
		StreamSetting m_setting;
		SRTPSocket m_srtpSocket;
		ServerSocket m_server;	//Listen if there is any subscriber
		uint8_t m_numSubscribers;
		bool m_isStreaming;
		pthread_mutex_t m_mutex;

		char m_srtpKey[MAX_SRTP_KEY_LENGTH];
		uint32_t m_currentTS;

		bool interrupt;
	};
}

#endif