#ifndef OPPVS_STUN_SOCKET_HPP
#define OPPVS_STUN_SOCKET_HPP

#include "physical_socket.hpp"
#include "stun_header.hpp"
#include "stun_socket_address.hpp"

namespace oppvs
{
	class StunSocket : public PhysicalSocket
	{
	private:
		SocketRole m_role;

		void reset();
		int init(int socktype, const StunSocketAddress& addlocal, SocketRole role, bool fSetReuseFlag);

		
	public:
		StunSocket();
		int Close();

		SocketRole getRole() const;
		void setRole(SocketRole);

		int initUDP(const StunSocketAddress& addlocal, SocketRole role);

	};
}

#endif