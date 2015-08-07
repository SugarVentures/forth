#include "ice_stream.hpp"

namespace oppvs {
	IceStream::IceStream(NiceAgent* agent, guint streamid, guint ncomponents): m_streamID(streamid)
	{

	}

	IceStream::~IceStream()
	{

	}

	guint IceStream::getStreamID()
	{
		return m_streamID;
	}
} // oppvs