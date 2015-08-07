#include "ice_manager.hpp"
#include <iostream>

namespace oppvs {
	IceManager::IceManager() : m_agent(NULL), m_globalMainLoop(NULL), m_globalMainLoopThread(NULL)
	{
		
	}

	IceManager::~IceManager()
	{
		if (m_agent)
			g_object_unref(m_agent);

	    if (m_globalMainLoop)
	    	g_main_loop_unref(m_globalMainLoop);

	    delete m_globalMainLoopThread;

	    while (m_streams.empty())
	    {
	    	delete m_streams.back();
	    	m_streams.pop_back();
	    }
	}

	int IceManager::init(const IceServerInfo& stun, const IceServerInfo& turn)
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

	int IceManager::createStream(guint ncomponents)
	{
		guint stream_id = nice_agent_add_stream(m_agent, ncomponents);
		if (stream_id == 0)
			return -1;

		std::cout << "Ice Stream id: " << stream_id << std::endl;
		IceStream *stream = new IceStream(m_agent, stream_id, ncomponents);
		m_streams.push_back(stream);	

		// Attach to the component to receive the data
	    for (guint component_id = 1; component_id <= ncomponents; component_id++ )
	        nice_agent_attach_recv(m_agent, stream_id, component_id, g_main_loop_get_context(m_globalMainLoop), 
	        	cb_nice_recv, (gpointer)stream);

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

	    // Start gathering local candidates
	    //if (!nice_agent_gather_candidates(m_agent, stream_id))
	    //	std::cout << "Failed to gather candidates" << std::endl;

		return 0;
	}

	int IceManager::removeStream(guint streamid)
	{
		if (getStreamByID(streamid) < 0)
		{
			std::cout << "Not found stream id " << streamid << std::endl;
			return -1;
		}
		nice_agent_remove_stream(m_agent, streamid);
		return 0;
	}

	int IceManager::getStreamByID(guint streamid)
	{
		std::vector<IceStream*>::const_iterator it;
		for (it = m_streams.begin(); it != m_streams.end(); ++it)
		{
			IceStream* stream = *it;
			if (stream->getStreamID() == streamid)
				return 0;
		}
		return -1;
	}

	void IceManager::cb_candidate_gathering_done( NiceAgent *agent, guint stream_id, gpointer user_data )
	{
		std::cout << "Gather candidate done" << std::endl;
	}
    
    void IceManager::cb_new_selected_pair( NiceAgent *agent, guint stream_id, guint component_id,
                                      gchar *lfoundation, gchar *rfoundation, gpointer user_data )
    {

    	
    }

    void IceManager::cb_nice_recv( NiceAgent *agent, guint stream_id, guint component_id,
                              guint len, gchar *buf, gpointer user_data )
    {

    }

    void IceManager::cb_component_state_changed(NiceAgent *agent, guint _stream_id, 
    						guint component_id, guint state, gpointer data)
    {

    }

    void* IceManager::runGlobalMainloop(void* arg)
    {
    	std::cout<< "Run global main loop" << std::endl;
    	GMainLoop *global_mainloop = (GMainLoop *)arg;
    	g_main_loop_run(global_mainloop);
    	return NULL;
    }

} // oppvs
