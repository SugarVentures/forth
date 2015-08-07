#ifndef OPPVS_ICE_STREAM_HPP
#define OPPVS_ICE_STREAM_HPP

#include "nice/nice.h"

namespace oppvs
{
	class IceStream
	{
	public:
		IceStream(NiceAgent* agent, guint streamid, guint ncomponents);
		~IceStream();

		guint getStreamID();
	private:
		guint m_streamID;
	};
}

#endif // OPPVS_ICE_STREAM_HPP
