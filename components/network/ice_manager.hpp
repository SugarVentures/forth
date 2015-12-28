#ifndef OPPVS_ICE_MANAGER_HPP
#define OPPVS_ICE_MANAGER_HPP

#include "nice/nice.h"
#include "gio/gnetworking.h"

#include "ice_common.hpp"
#include "ice_stream.hpp"
#include "thread.hpp"

#include <vector>

namespace oppvs {
	typedef void (*callbackCandidateGatheringDone)(void* object, void* icemgr, std::string username, std::string password, std::vector<IceCandidate>& candidates);
	typedef void (*callbackNewSubscriber)(void* object, IceStream* stream);
	typedef void (*callbackOnReceive)(void* object, uint8_t* data, uint32_t len);
	
	class IceManager {
	public:
		IceManager();
		IceManager(void* object);
		~IceManager();

		int init(const IceServerInfo& stun, const IceServerInfo& turn, int mode);
		int release();
		IceStream* createStream(guint ncomponents = 1);
		IceStream* getStreamByID(guint streamid);
		int removeStream(guint streamid);

		void setPeerInfo(const std::string& username, const std::string& password, std::vector<IceCandidate>& candidates);
		void establishPeerConnection(guint streamid = 1);

		void attachCallbackEvent(callbackCandidateGatheringDone cb, void* object);
		void attachCallbackEvent(callbackNewSubscriber cb, void* object);
		void attachCallbackEvent(callbackOnReceive cb, void* object);

	private:
		NiceAgent* m_agent;
		IceServerInfo m_stunServer;
		IceServerInfo m_turnServer;
		GMainLoop* m_globalMainLoop;
		Thread* m_globalMainLoopThread;

		std::vector<IceStream*> m_streams;
		std::vector<IceCandidate> m_remoteCandidates;
		std::string m_remoteUsername;
		std::string m_remotePassword;

		
		static void cb_candidate_gathering_done( NiceAgent *agent, guint stream_id, gpointer user_data );
    	static void cb_new_selected_pair( NiceAgent *agent, guint stream_id, guint component_id,
                                      gchar *lfoundation, gchar *rfoundation, gpointer user_data );
    	static void cb_nice_recv( NiceAgent *agent, guint stream_id, guint component_id,
                              guint len, gchar *buf, gpointer user_data );
    	static void cb_component_state_changed(NiceAgent *agent, guint stream_id, 
    						guint component_id, guint state, gpointer data);

    	static void *runGlobalMainloop(void* arg);

    	callbackCandidateGatheringDone cbCandidateGatheringDoneEvent;
    	void* cbCandidateGatheringDoneObject;

    	callbackNewSubscriber cbNewSubscriberEvent;
    	void* cbNewSubscriberObject;

    	callbackOnReceive cbOnReceiveEvent;
    	void* cbOnReceiveObject;

	};
} // oppvs

#endif // OPPVS_ICE_MANAGER_HPP
