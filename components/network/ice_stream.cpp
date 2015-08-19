#include "ice_stream.hpp"
#include <iostream>

namespace oppvs {
	IceStream::IceStream(NiceAgent* agent, guint streamid, guint ncomponents): m_streamID(streamid), m_agent(agent), m_noComponents(ncomponents)
	{
		gchar *username = NULL;
		gchar *password = NULL;
	    if (nice_agent_get_local_credentials(agent, streamid, &username, &password))
	    {
		    m_localUsername = username;
		    m_localPassword = password;
		    if (username)
		    	g_free(username);
		    if (password)
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
		//nice_agent_remove_stream(m_agent, m_streamID);
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

    void IceStream::setRemoteCandidates(std::vector<IceCandidate> rcands)
	{
	    // Get the list of lists of candidates grouped by component_id
	    std::vector<GSList*> cands = convertIceCandidateToNiceCandidate(rcands);

	    for (unsigned int i = 0; i < cands.size(); i++)
	    {
	        NiceCandidate* cand = (NiceCandidate*)cands[i]->data;
	        guint comp_id = cand->component_id;

	        std::cout << "Selecting for component " << comp_id << "..." << std::endl;
	        if (nice_agent_set_remote_candidates(m_agent, m_streamID, comp_id, cands[i]) < 1)
	        	std::cout << "Failed to set remote candidates of stream " << m_streamID << std::endl;

	        g_slist_free_full(cands[i], (GDestroyNotify)&nice_candidate_free);
	    }
	}

	void IceStream::setRemoteCredentials(const std::string& username, const std::string& password)
	{
		m_remoteUsername = username;
		m_remotePassword = password;
		if (!nice_agent_set_remote_credentials(m_agent, m_streamID, username.c_str(), password.c_str())) {
		    std::cout << "Failed to set remote credentials";
		    return;
		}
	}

    void IceStream::convertNiceCandidateToIceCandidate(GSList* cands, std::vector<IceCandidate>& candidates)
	{
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
	        candidate.port = port;
	        candidate.priority = cand->priority;
	        candidate.protocol = IceCandidateProtocol[cand->transport];
	        candidate.rel_addr = rel_addr;
	        candidate.rel_port = rel_port;
	        candidate.type = IceCandidateTypeName[cand->type];

	        candidates.push_back(candidate);
	    }
	}

	std::string IceStream::getLocalUsername() const
	{
		return m_localUsername;
	}

	std::string IceStream::getLocalPassword() const
	{
		return m_localPassword;
	}

	std::vector<GSList*> IceStream::convertIceCandidateToNiceCandidate(std::vector<IceCandidate> cands)
	{
		std::vector<GSList*> candidates;

	    for (unsigned int i = cands.size(); i > 0;)
	    {
	        i--;

	        NiceCandidateType type;
	        if (IceCandidateTypeName[NICE_CANDIDATE_TYPE_HOST] == cands[i].type)
	            type = NICE_CANDIDATE_TYPE_HOST;
	        if (IceCandidateTypeName[NICE_CANDIDATE_TYPE_SERVER_REFLEXIVE] == cands[i].type)
	            type = NICE_CANDIDATE_TYPE_SERVER_REFLEXIVE;
	        if (IceCandidateTypeName[NICE_CANDIDATE_TYPE_PEER_REFLEXIVE] == cands[i].type)
	            type = NICE_CANDIDATE_TYPE_PEER_REFLEXIVE;
	        if (IceCandidateTypeName[NICE_CANDIDATE_TYPE_RELAYED] == cands[i].type)
	            type = NICE_CANDIDATE_TYPE_RELAYED;

	        NiceCandidate* candidate = nice_candidate_new(type);

	        candidate->transport = NICE_CANDIDATE_TRANSPORT_UDP;

	        nice_address_init(&candidate->addr);
	        nice_address_set_from_string(&candidate->addr, cands[i].ip.c_str());
	        nice_address_set_port(&candidate->addr, cands[i].port);

	        if (cands[i].rel_addr != "")
	        {
		        nice_address_init(&candidate->base_addr);
		        nice_address_set_from_string(&candidate->base_addr, cands[i].rel_addr.c_str());
		        nice_address_set_port(&candidate->base_addr, cands[i].rel_port);
		    }

	        candidate->priority = cands[i].priority;
	        candidate->component_id = 1;	//Todo: support multiple components
	        candidate->stream_id = m_streamID;
	        std::strncpy(candidate->foundation, cands[i].foundation.c_str(), NICE_CANDIDATE_MAX_FOUNDATION);
	        candidate->foundation[NICE_CANDIDATE_MAX_FOUNDATION - 1] = 0;
	        
	        unsigned int candidates_size = candidates.size();

	        for (unsigned int j = 0; j <= candidates_size; j++)
	        {
	            if (j == candidates_size)
	            {
	                candidates.push_back(NULL);
	                candidates[j] = g_slist_prepend(candidates[j], candidate);
	            }
	            else
	            {
	                NiceCandidate* cand = (NiceCandidate*)candidates[j]->data;
	                if (candidate->component_id == cand->component_id)
	                {
	                    candidates[j] = g_slist_prepend(candidates[j], candidate);
	                    break;
	                }
	            }
	        }
	    }

	    return candidates;
	}

	void IceStream::send(uint16_t size, uint8_t* data, guint component_id)
	{
		nice_agent_send(m_agent, m_streamID, component_id, size, (gchar*)data);
	}

    void IceStream::receive(guint size, gchar* data, guint component_id)
    {
    	printf("Receive data size %u\n", size);    	
    }

    
} // oppvs