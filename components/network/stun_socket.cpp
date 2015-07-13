#include "stun_socket.hpp"

#include <assert.h>

namespace oppvs
{
	StunSocket::StunSocket() : m_role(RolePP)
	{

	}

	void StunSocket::reset()
	{
		m_role = RolePP;
	}

	int StunSocket::Close()
	{
		int result = PhysicalSocket::Close();
        reset();
		return result;
	}

	void StunSocket::setRole(SocketRole role)
	{
		m_role = role;
	}

	SocketRole StunSocket::getRole() const
	{
		return m_role;
	}

	int StunSocket::init(int socktype, const StunSocketAddress& addlocal, SocketRole role, bool fSetReuseFlag)
	{
		assert((socktype == SOCK_DGRAM) || (socktype == SOCK_STREAM));
		if (Create(addlocal.getIP().getAddressFamily(), socktype, 0) < 0)
			return -1;

		if (Bind((SocketAddress)addlocal) < 0)
			return -1;
		printf("Socket fd: %d %s\n", m_socketfd, getLocalAddress().toString().c_str());
		setRole(role);
		return 0;
	}

	int StunSocket::initUDP(const StunSocketAddress& addlocal, SocketRole role)
	{
		return init(SOCK_DGRAM, addlocal, role, false);
	}

	bool StunSocket::isValid()
	{
		return (m_socketfd != -1);
	}
}
