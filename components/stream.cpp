/*
*	Streamming module
*/

#include "interface/stream.hpp"
#include "errno.h"
#include <pthread.h>

namespace oppvs
{

	Frame::Frame() : m_data(NULL), m_length(0), m_timestamp(0)
	{	

	}

	Frame::Frame(const PixelBuffer& pixelbuf, uint32_t ts) : m_data(NULL), m_length(0), m_timestamp(0)
	{
		if (pixelbuf.nbytes == 0)
			return;
		m_length = 6 + pixelbuf.nbytes;	//width (2 bytes), height (2 bytes), stride (2 bytes)
		m_data = new uint8_t[m_length];
		m_timestamp = ts;
		memcpy(m_data, &pixelbuf.width[0], 2);
		memcpy(m_data + 2, &pixelbuf.height[0], 2);
		memcpy(m_data + 4, &pixelbuf.stride[0], 2);
		memcpy(m_data + 6, &pixelbuf.flip, 1);
		memcpy(m_data + 7, pixelbuf.plane[0], m_length);
	}

	Frame::~Frame()
	{
		if (m_data != NULL)
			delete [] m_data;
		m_length = 0;
		m_timestamp = 0;
		m_data = NULL;
	}

	Stream::Stream() : m_maxSendingQueueLength(OPPVS_MAX_SENDING_QUEUE_LENGTH), m_numSubscribers(0), m_isStreaming(false),
		m_currentTS(0), interrupt(0)
	{
		pthread_mutex_init(&m_mutex, NULL);
	}

	Stream::Stream(const StreamSetting& setting) : m_maxSendingQueueLength(OPPVS_MAX_SENDING_QUEUE_LENGTH), m_numSubscribers(0),
	m_isStreaming(false), m_currentTS(0), interrupt(0)
	{
		pthread_mutex_init(&m_mutex, NULL);
		m_setting = setting;
	}

	Stream::~Stream()
	{
		Frame *frame;
		interrupt = true;
		while (!m_sendingQueue.empty())
		{
			frame = m_sendingQueue.front();
			m_sendingQueue.pop();
			delete frame;
		}
		m_srtpSocket.Close();
		m_server.Close();
	}

	void* sendData(void *svr)
	{
		Stream* stream = (Stream*)svr;
		ConQueue<Frame*>* queue = stream->getQueue();
		Frame *ptr_frame;
		Frame frame;
		while (!queue->empty())
		{
			//queue->lockpop();
			ptr_frame = queue->front();
			frame = *ptr_frame;
			queue->try_pop(ptr_frame);
			delete ptr_frame;
			//queue->unlockpop();
			uint32_t ts = frame.getTimeStamp();
			int msgLength = frame.getLength();
			int sendLength = msgLength > OPPVS_NETWORK_PACKET_LENGTH ? OPPVS_NETWORK_PACKET_LENGTH : msgLength;
			uint8_t* curPos = frame.getData();
			

			uint count = 0;
			while (msgLength > 0)
			{
				//memset(curPos, 1, sendLength);
				int len = stream->getStreamingSocket()->SendTo(curPos, sendLength, ts);
				if (len < 0)
				{
					printf("Send failed %s\n", strerror(errno));
					break;
				}
				curPos += sendLength;
				msgLength -= sendLength;
				count++;
				sendLength = msgLength > OPPVS_NETWORK_PACKET_LENGTH ? OPPVS_NETWORK_PACKET_LENGTH : msgLength;
				usleep(10);
			}
			//printf("Number of packet per frame: %d total bytes: %u\n", count, frame.getLength());

			
		}
		//stream->setIsStreaming(true);
		pthread_exit(NULL);
	}

	int Stream::pushData(const PixelBuffer& pixelbuf)
	{
		Frame* frame;
		if (m_sendingQueue.size() >= m_maxSendingQueueLength)
		{
			frame = m_sendingQueue.front();
			m_sendingQueue.pop();
			delete frame;
		}
		frame = new Frame(pixelbuf, m_currentTS);
		m_sendingQueue.push(frame);
		m_currentTS++;
		if (m_isStreaming)
		{
			pthread_t thread;			
			int rc = pthread_create(&thread, NULL, sendData, (void*)this);
			if (rc )
			{
				printf("Unable to create thread for server \n");
				return -1;
			}
			m_isStreaming = false;
		}
		return 0;
	}

	

	int Stream::initStreamingSocket(const SocketAddress& remote)
	{
		IPAddress ipAddr;
		SocketAddress socketAddr;
		socketAddr.setIP(ipAddr);
		socketAddr.setPort(m_setting.port + m_numSubscribers);

		m_srtpSocket.initSRTPLib();
		if (m_srtpSocket.Create(AF_INET, SOCK_DGRAM, IPPROTO_UDP) < 0)
			return -1;
		if (m_srtpSocket.Bind(socketAddr) < 0)
			return -1;

		SocketAddress remoteAddr;
		remoteAddr.setIP(remote.getIP());
		printf("Local Port: %d, Remote port: %d\n", m_setting.port + m_numSubscribers, m_setting.port + 1);
		remoteAddr.setPort(m_setting.port + 1);
		if (m_srtpSocket.Accept(remoteAddr) < 0)
			return -1;

		m_srtpSocket.setPolicy(m_srtpKey);
		m_srtpSocket.initSender();
		return 0;
	}

	void* startListening(void *svr)
	{
		Stream* stream = (Stream*)svr;
		ServerSocket* server = stream->getServer();
		if (server->Listen() < 0)
		{
			printf("Error when creating signaling server\n");
			return NULL;
		}
		while (!stream->isInterrupt())
		{
			SocketAddress remoteAddr;
			int sockfd = server->Accept(remoteAddr);
			if (sockfd >= 0)
			{
				printf("New subscriber \n");
				server->increaseNumSubscribers();				
				if (stream->initStreamingSocket(remoteAddr) < 0)
				{
					printf("Init srtp failed\n");
					return NULL;
				}
				stream->setIsStreaming(true);
				server->Close(sockfd);
			}
			usleep(OPPVS_IDLE_TIME);
		}
		server->Close();
		pthread_exit(NULL);
	}


	int Stream::initServer()
	{
		pthread_t thread;

		IPAddress ipAddr;
		SocketAddress socketAddr;
		socketAddr.setIP(ipAddr);
		socketAddr.setPort(m_setting.port);
		printf("Init server at port %d\n", m_setting.port);

		if (m_server.Create(AF_INET, SOCK_STREAM, 0) < 0)
		{
			printf("Open socket error %s\n", strerror(errno));
			return -1;
		}
		if (m_server.Bind(socketAddr) < 0)
		{
			printf("Binding error %s\n", strerror(errno));
			return -1;
		}

		int rc = pthread_create(&thread, NULL, startListening, (void*)this);

		if (rc )
		{
			printf("Unable to create thread for server \n");
			return -1;
		}
		
		return 0;
	}

	ServerSocket* Stream::getServer()
	{
		return &m_server;
	}

	void Stream::setIsStreaming(bool value)
	{
		m_isStreaming = value;
	}
}