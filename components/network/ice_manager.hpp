#ifndef OPPVS_ICE_MANAGER_HPP
#define OPPVS_ICE_MANAGER_HPP

#include "nice/nice.h"
#include "gio/gnetworking.h"

#include "ice_common.hpp"
#include "thread.hpp"

namespace oppvs {
	class IceManager {
	public:
		IceManager();
		~IceManager();

		int init(const IceServerInfo& stun, const IceServerInfo& turn);
		int release();

	private:
		NiceAgent* m_agent;
		IceServerInfo m_stunServer;
		IceServerInfo m_turnServer;
		GMainLoop* m_globalMainLoop;
		Thread* m_globalMainLoopThread;

		static void cb_candidate_gathering_done( NiceAgent *agent, guint stream_id, gpointer user_data );
    	static void cb_new_selected_pair( NiceAgent *agent, guint stream_id, guint component_id,
                                      gchar *lfoundation, gchar *rfoundation, gpointer user_data );
    	static void cb_nice_recv( NiceAgent *agent, guint stream_id, guint component_id,
                              guint len, gchar *buf, gpointer user_data );
    	static void cb_component_state_changed(NiceAgent *agent, guint _stream_id, 
    						guint component_id, guint state, gpointer data);

    	static void *runGlobalMainloop(void* arg);

	};
} // oppvs

#endif // OPPVS_ICE_MANAGER_HPP
