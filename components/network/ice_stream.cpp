#include "ice_stream.hpp"
#include <iostream>

namespace oppvs {
	IceStream::IceStream(NiceAgent* agent, guint streamid, guint ncomponents): m_streamID(streamid), m_agent(agent), m_noComponents(ncomponents)
	{
		gchar *username, *password;
	    if (nice_agent_get_local_credentials(agent, streamid, &username, &password))
	    {
		    m_localUsername = username;
		    m_localPassword = password;
		    g_free(username);
		    g_free(password);
		}
		else
		{
			m_localUsername = "";
			m_localPassword = "";
		}
	}

	IceStream::~IceStream()
	{
		nice_agent_remove_stream(m_agent, m_streamID);
	}

	guint IceStream::getStreamID()
	{
		return m_streamID;
	}

	int IceStream::requestLocalCandidatesFromComponent(guint component_id)
	{
		if (!nice_agent_gather_candidates(m_agent, m_streamID))
			return -1;
		return 0;
	}

    int IceStream::requestLocalCandidates()
    {
	    for (guint component_id = 1; component_id <= m_noComponents; component_id++ )
	    {
	        if (requestLocalCandidatesFromComponent(component_id) < 0)
	        	return -1;
	    }

	    return 0;
    }

    std::vector<IceCandidate> IceStream::convertNiceCandidateToIceCandidate(GSList* cands)
	{
	    std::vector<IceCandidate> candidates;
	    char ip[INET6_ADDRSTRLEN];
	    char rel_addr[INET6_ADDRSTRLEN];

	    for ( ; cands; cands = cands->next )
	    {
	        NiceCandidate* cand = (NiceCandidate*)cands->data;

	        nice_address_to_string(&cand->addr, ip);
	        unsigned int port = nice_address_get_port(&cand->addr);

	        nice_address_to_string(&cand->base_addr, rel_addr );
	        unsigned int rel_port = nice_address_get_port( &cand->base_addr );

	        IceCandidate candidate;

	        candidate.component = cand->component_id;
	        candidate.foundation = cand->foundation;
	        candidate.ip = ip;
	        candidate.network = 0; // FIXME
	        candidate.port = port;
	        candidate.priority = cand->priority;
	        //candidate.protocol = candidate_protocol[cand->transport];
	        candidate.rel_addr = rel_addr;
	        candidate.rel_port = rel_port;
	        //candidate.type = candidate_type[cand->type];

	        std::cout << "Candidate: " << candidate.component << " "
	     			  << candidate.foundation << " "
	     			  << ip << " "
	     			  << port << std::endl;

	        candidates.push_back(candidate);
	    }

	    return candidates;
	}

	std::string IceStream::getLocalUsername() const
	{
		return m_localUsername;
	}

	std::string IceStream::getLocalPassword() const
	{
		return m_localPassword;
	}
} // oppvs