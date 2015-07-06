#include "stun_socket.hpp"

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
}