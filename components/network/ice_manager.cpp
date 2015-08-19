#include "ice_manager.hpp"
#include <iostream>

namespace oppvs {
	IceManager::IceManager() : m_agent(NULL), m_globalMainLoop(NULL), m_globalMainLoopThread(NULL)
	{
		cbCandidateGatheringDoneObject = NULL;
		cbOnReceiveObject = NULL;
		m_remoteUsername = "";
		m_remotePassword = "";
	}

	IceManager::IceManager(void* object) : m_agent(NULL), m_globalMainLoop(NULL), m_globalMainLoopThread(NULL)
	{

	}

	IceManager::~IceManager()
	{
		if (m_agent)
			g_object_unref(m_agent);

	    if (m_globalMainLoop)
	    	g_main_loop_unref(m_globalMainLoop);

	    delete m_globalMainLoopThread;

	    while (m_streams.size() > 0)
	    {
	    	delete m_streams.back();
	    	m_streams.pop_back();
	    }
	}

	int IceManager::init(const IceServerInfo& stun, const IceServerInfo& turn, int mode)
	{
		m_stunServer = stun;
		m_turnServer = turn;
		g_networking_init();

		m_globalMainLoop = g_main_loop_new(NULL, FALSE);

		m_agent = nice_agent_new(g_main_loop_get_context (m_globalMainLoop), NICE_COMPATIBILITY_RFC5245);
  
  		if (m_agent == NULL)
  		{
  			std::cout << "Failed to create nice agent." << std::endl;
  			return -1;
  		}

  		if (m_stunServer.serverAddress != "")
  		{
  			g_object_set(m_agent, "stun-server", m_stunServer.serverAddress.c_str(), NULL);
    		g_object_set(m_agent, "stun-server-port", m_stunServer.port, NULL);
  		}

  		g_object_set(m_agent, "controlling-mode", mode, NULL);

  		// Connect to the signals
		g_signal_connect(m_agent, "candidate-gathering-done", G_CALLBACK(cb_candidate_gathering_done), (gpointer)this);
		g_signal_connect(m_agent, "new-selected-pair", G_CALLBACK(cb_new_selected_pair), (gpointer)this);
		g_signal_connect(m_agent, "component-state-changed", G_CALLBACK(cb_component_state_changed), (gpointer)this);

		m_globalMainLoopThread = new Thread(IceManager::runGlobalMainloop, (void*)m_globalMainLoop);
		m_globalMainLoopThread->create();
  		return 0;
	}

	int IceManager::release()
	{
		g_main_loop_quit(m_globalMainLoop);
		return 0;
	}

	IceStream* IceManager::createStream(guint ncomponents)
	{
		guint stream_id = nice_agent_add_stream(m_agent, ncomponents);
		if (stream_id == 0)
			return NULL;

		std::cout << "Ice Stream id: " << stream_id << std::endl;
		IceStream *stream = new IceStream(m_agent, stream_id, ncomponents);
		m_streams.push_back(stream);	

		// Attach to the component to receive the data
	    for (guint component_id = 1; component_id <= ncomponents; component_id++ )
	        nice_agent_attach_recv(m_agent, stream_id, component_id, g_main_loop_get_context(m_globalMainLoop), 
	        	cb_nice_recv, (gpointer)this);

	    // Setting turn server properties
	    if (m_turnServer.serverAddress != "")
	    {
	        std::cout << "Setup TURN server info" << std::endl;
	        for (guint component_id = 1; component_id <= ncomponents; component_id++)
	        {
	            nice_agent_set_relay_info(m_agent, stream_id, component_id, m_turnServer.serverAddress.c_str(), m_turnServer.port,
	                                       m_turnServer.username.c_str(), m_turnServer.password.c_str(), NICE_RELAY_TYPE_TURN_UDP);
	        }
	    }

		return stream;
	}

	int IceManager::removeStream(guint streamid)
	{
		if (getStreamByID(streamid) != NULL)
		{
			std::cout << "Not found stream id " << streamid << std::endl;
			return -1;
		}
		nice_agent_remove_stream(m_agent, streamid);
		return 0;
	}

	IceStream* IceManager::getStreamByID(guint streamid)
	{
		std::vector<IceStream*>::const_iterator it;
		for (it = m_streams.begin(); it != m_streams.end(); ++it)
		{
			IceStream* stream = *it;
			if (stream->getStreamID() == streamid)
				return stream;
		}
		return NULL;
	}

	void IceManager::cb_candidate_gathering_done(NiceAgent *agent, guint stream_id, gpointer user_data)
	{
		std::cout << "Gather candidate done" << std::endl;
		IceManager* manager = (IceManager*)user_data;
		IceStream* stream = manager->getStreamByID(stream_id);
		GSList *cands = NULL;
		cands = nice_agent_get_local_candidates(agent, stream_id, 1);
		if (cands)
		{
			std::vector<IceCandidate> candidates;
			stream->convertNiceCandidateToIceCandidate(cands, candidates);
   			g_slist_free_full(cands, (GDestroyNotify)&nice_candidate_free);

   			if (manager->cbCandidateGatheringDoneObject != NULL)
	   			manager->cbCandidateGatheringDoneEvent(manager->cbCandidateGatheringDoneObject, (void*)manager,
	   				stream->getLocalUsername(), stream->getLocalPassword(), candidates);
   		}

	}
    
    void IceManager::cb_new_selected_pair(NiceAgent *agent, guint stream_id, guint component_id,
                                      gchar *lfoundation, gchar *rfoundation, gpointer user_data)
    {
    	IceManager* manager = (IceManager*)user_data;
    	IceStream* stream = manager->getStreamByID(stream_id);
    	if (stream != NULL)
    		manager->cbNewSubscriberEvent(manager->cbNewSubscriberObject, stream);
    }

    void IceManager::cb_nice_recv(NiceAgent *agent, guint stream_id, guint component_id,
                              guint len, gchar *buf, gpointer user_data)
    {
	   	IceManager* manager = (IceManager*)user_data;
		manager->cbOnReceiveEvent(manager->cbOnReceiveObject, (uint8_t*)buf, len);
    }

    void IceManager::cb_component_state_changed(NiceAgent *agent, guint stream_id, 
    						guint component_id, guint state, gpointer data)
    {
    	std::cout  << "SIGNAL: state changed Stream: " << stream_id
    			   << " Component ID: " << component_id
    			   << " State: " << IceStateName[state] << std::endl;
    	if (state == NICE_COMPONENT_STATE_READY) {
    		NiceCandidate *local, *remote;
    		if (nice_agent_get_selected_pair(agent, stream_id, component_id,
                &local, &remote))
            {
            	gchar ipaddr[INET6_ADDRSTRLEN];
				nice_address_to_string(&local->addr, ipaddr);
				printf("\nNegotiation complete: ([%s]:%d,",
				 	ipaddr, nice_address_get_port(&local->addr));
				nice_address_to_string(&remote->addr, ipaddr);
				printf(" [%s]:%d)\n", ipaddr, nice_address_get_port(&remote->addr));

    		}
    	}
    	else if (state == NICE_COMPONENT_STATE_FAILED) {
    		
  		}
    }

    void* IceManager::runGlobalMainloop(void* arg)
    {
    	std::cout<< "Run global Ice main loop" << std::endl;
    	GMainLoop *global_mainloop = (GMainLoop *)arg;
    	g_main_loop_run(global_mainloop);
    	return NULL;
    }

    void IceManager::attachCallbackEvent(callbackCandidateGatheringDone cb, void* object)
    {
    	cbCandidateGatheringDoneEvent = cb;
    	cbCandidateGatheringDoneObject = object;
    }

    void IceManager::attachCallbackEvent(callbackNewSubscriber cb, void* object)
    {
    	cbNewSubscriberEvent = cb;
    	cbNewSubscriberObject = object;
    }

    void IceManager::attachCallbackEvent(callbackOnReceive cb, void* object)
    {
    	cbOnReceiveObject = object;
    	cbOnReceiveEvent = cb;
    }

    void IceManager::setPeerInfo(const std::string& username, const std::string& password, std::vector<IceCandidate>& candidates)
    {
    	m_remoteUsername = username;
    	m_remotePassword = password;
    	m_remoteCandidates = candidates;
    }

    void IceManager::establishPeerConnection()
    {
    	IceStream* stream = getStreamByID(1);
		stream->setRemoteCredentials(m_remoteUsername, m_remotePassword);
		stream->setRemoteCandidates(m_remoteCandidates);
    }

} // oppvs
