/*
	Network stream: send/receive raw streaming data from network
*/

#ifndef OPPVS_NETWORK_STREAM_HPP
#define OPPVS_NETWORK_STREAM_HPP

#include "network.hpp"
#include "concurrent_queue.hpp"

#include <map>

namespace oppvs
{

	enum NetworkRole
	{
		SENDER_ROLE,
		RECEIVER_ROLE
	};

	struct RawData
	{
		RawData()
		{
			data = NULL;
			length = 0;
			count = 0;
		}

		RawData(const uint8_t* dt, uint32_t len, int8_t cnt)
		{
			data = new uint8_t[len];
			memcpy(data, dt, len);
			length = len;
			count = cnt;
		}
		~RawData()
		{
			delete [] data;
		}
		uint32_t length;
		uint8_t* data;
		int8_t count;
		uint16_t width;
		uint16_t height;
		uint8_t sourceid;
		uint8_t order;
	};

	//Size of the FrameBegin must be odd
	struct FrameBegin
	{
		uint8_t flag;
		uint8_t source;
		uint8_t order;
		uint16_t width;
		uint16_t height;
		

		inline int size()
		{
			return sizeof(FrameBegin);
		}
	};

	struct FrameEnd
	{
		uint8_t flag;
	};

	struct FrameInfo
	{
		uint16_t width;
		uint16_t height;
		uint16_t originx;
		uint16_t originy;
		uint8_t source;
		uint8_t order;
	};


	typedef void (*on_send_done_event)(void* owner, int error);
	typedef void (*on_receive_event)(void* ownver, int error);

	class NetworkStream
	{
	public:
		NetworkStream(NetworkRole role, uint32_t ssrc);
		int setup(uint32_t port);
		void setSender(const SocketAddress& dest);
		void setReceiver(const SocketAddress& dest);
		void releaseSender();
		void releaseReceiver();

		virtual ~NetworkStream() {}
		int write(const uint8_t* data, uint32_t length, uint32_t* written);
		int read(uint8_t* &buffer, uint32_t length, uint32_t* read, FrameInfo& info);

		void sendStream();
		void waitStream();

		void registerCallback(void* owner, void* squeue, on_send_done_event event);
		void registerCallback(void* owner, PixelBuffer* pf, on_receive_event event);
		SocketAddress& getLocalAddress();

		void unlock();
	private:
		NetworkRole m_role;
		SRTPSocket m_socket;
		char m_srtpKey[MAX_SRTP_KEY_LENGTH];
		
		const static int MAX_RETRY_TIMES = 3;
		const static int EXPIRE_TIME = 5;
		int m_timestamp;
		bool m_busy;
		int m_error;

		void* m_owner;
		on_send_done_event m_sendDoneEvent;
		on_receive_event m_receiveEvent;

		ConQueue<RawData*> *p_sendingQueue;
		PixelBuffer* m_buffer;

		void sendDone(int* error);

		void addSource(FrameInfo& inf);
	};
}

#endif