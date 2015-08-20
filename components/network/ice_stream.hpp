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
		void setRemoteCredentials(const std::string& username, const std::string& password);
		void setRemoteCandidates(std::vector<IceCandidate> rcands);

		void convertNiceCandidateToIceCandidate(GSList* cands, std::vector<IceCandidate>& results);
		std::vector<GSList*> convertIceCandidateToNiceCandidate(std::vector<IceCandidate> cands);

		std::string getLocalUsername() const;
		std::string getLocalPassword() const;

		void send(uint16_t size, uint8_t* data, guint component_id = 1);

    
	private:
		guint m_streamID;
		NiceAgent* m_agent;
		guint m_noComponents;
    
		std::string m_localUsername;
		std::string m_localPassword;
		std::string m_remoteUsername;
		std::string m_remotePassword;

	};
}

#endif // OPPVS_ICE_STREAM_HPP
