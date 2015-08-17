#ifndef OPPVS_ICE_MANAGER_HPP
#define OPPVS_ICE_MANAGER_HPP

#include "nice/nice.h"
#include "gio/gnetworking.h"

#include "ice_common.hpp"
#include "ice_stream.hpp"
#include "thread.hpp"

#include <vector>

namespace oppvs {
	typedef void (*callbackCandidateGatheringDone)(void* object, std::string username, std::string password, std::vector<IceCandidate>& candidates);
	typedef void (*callbackNewSubscriber)(void* object, IceStream* stream);
	

	class IceManager {
	public:
		IceManager();
		IceManager(void* object);
		~IceManager();

		int init(const IceServerInfo& stun, const IceServerInfo& turn);
		int release();
		IceStream* createStream(guint ncomponents = 1);
		IceStream* getStreamByID(guint streamid);
		int removeStream(guint streamid);
		void registerCallback(callbackCandidateGatheringDone cb);
		void registerChannelCallback(callbackNewSubscriber cb, void* channel);
		void registerCallback(callbackOnReceive cb, void* object);
		void* getCallbackObject();

		void attachCallbackEvent(callbackCandidateGatheringDone cb, void* object);

		callbackCandidateGatheringDone cbCandidateGatheringDone;
		callbackNewSubscriber cbNewSubscriber;
		void* m_channel;

		callbackOnReceive cbOnReceive;
		void* rcvObject;

		callbackCandidateGatheringDone cbCandidateGatheringDoneEvent;
    	void*	cbCandidateGatheringDoneObject;

	private:
		NiceAgent* m_agent;
		IceServerInfo m_stunServer;
		IceServerInfo m_turnServer;
		GMainLoop* m_globalMainLoop;
		Thread* m_globalMainLoopThread;

		std::vector<IceStream*> m_streams;
		void* m_cbObject;

		
		static void cb_candidate_gathering_done( NiceAgent *agent, guint stream_id, gpointer user_data );
    	static void cb_new_selected_pair( NiceAgent *agent, guint stream_id, guint component_id,
                                      gchar *lfoundation, gchar *rfoundation, gpointer user_data );
    	static void cb_nice_recv( NiceAgent *agent, guint stream_id, guint component_id,
                              guint len, gchar *buf, gpointer user_data );
    	static void cb_component_state_changed(NiceAgent *agent, guint stream_id, 
    						guint component_id, guint state, gpointer data);

    	static void *runGlobalMainloop(void* arg);

    	
	};
} // oppvs

#endif // OPPVS_ICE_MANAGER_HPP
