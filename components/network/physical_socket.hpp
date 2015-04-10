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
		virtual int Accept(const SocketAddress& remote);

		virtual int Send(int destsock, void* msg, uint16_t len);
		virtual int Receive(void* msg, uint16_t len);
		virtual int Close(int destsock);
		void setReceiveTimeOut(int expire);
	
	protected:
		int m_socketfd;
		SocketAddress m_localAddress;
		SocketAddress m_remoteAddress;
	private:
		
		int m_backlog;
	};
}

#endif