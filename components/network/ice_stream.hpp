#ifndef OPPVS_ICE_STREAM_HPP
#define OPPVS_ICE_STREAM_HPP

#include "nice/nice.h"
#include "ice_common.hpp"
#include <vector>
#include <cstring>

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

		std::string getLocalUsername() const;
		std::string getLocalPassword() const;
	private:
		guint m_streamID;
		NiceAgent* m_agent;
		guint m_noComponents;
    
		std::string m_localUsername;
		std::string m_localPassword;
	};
}

#endif // OPPVS_ICE_STREAM_HPP
