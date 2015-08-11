/*
	Socket common interfaces
*/

#ifndef OPPVS_PHYSICAL_SOCKET_HPP
#define OPPVS_PHYSICAL_SOCKET_HPP


#include "socket_address.hpp"
#include <sys/socket.h>
#include <unistd.h>		/* for close () socket function */

namespace oppvs
{
	class PhysicalSocket
	{
	public:

		PhysicalSocket() : m_socketfd(-1), m_backlog(5) {}
		virtual ~PhysicalSocket() {}

		int Bind(const SocketAddress& addr);
		int Create(int family, int type, int protocol);
		virtual int Close();
		virtual int Listen();
		virtual int Connect(const SocketAddress& dest);
		virtual int Accept(SocketAddress& remote);

		virtual int Send(int destsock, void* msg, uint16_t len);
		virtual int Send(int sockfd, void* msg, uint16_t len, const SocketAddress& dest);
		virtual int Receive(void* msg, uint16_t len);
		virtual int Receive(int sockfd, void* msg, uint16_t len);
		virtual int ReceiveMsg(int sockfd, void* buf, uint16_t len, int flags, SocketAddress& src, SocketAddress& dest);
		virtual int Close(int destsock);

		void setReceiveTimeOut(int expire);
		SocketAddress& getLocalAddress();
		SocketAddress& getRemoteAddress();
		int getSocketHandle();

		int enablePacketInfoOption(bool enable);
		bool isValid();
	protected:
		int m_socketfd;
		SocketAddress m_localAddress;
		SocketAddress m_remoteAddress;
	private:
		int enablePacketInfoOptionCommon(int level, int option1, int option2, bool enable);
		int enablePacketInfoOptionIpv4(bool enable);
		int enablePacketInfoOptionIpv6(bool enable);
	
		int m_backlog;
	};
}

#endif