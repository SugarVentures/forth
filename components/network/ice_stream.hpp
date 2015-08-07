#ifndef OPPVS_ICE_STREAM_HPP
#define OPPVS_ICE_STREAM_HPP

#include "nice/nice.h"
#include "ice_common.hpp"
#include <vector>

namespace oppvs
{
	class IceStream
	{
	public:
		IceStream(NiceAgent* agent, guint streamid, guint ncomponents);
		~IceStream();

		guint getStreamID();
		
		int requestLocalCandidatesFromComponent(guint component_id = 1);
		int requestLocalCandidates();

		std::vector<IceCandidate> convertNiceCandidateToIceCandidate(GSList* cands);
	private:
		guint m_streamID;
		NiceAgent* m_agent;
		guint m_noComponents;
    
		
	};
}

#endif // OPPVS_ICE_STREAM_HPP
