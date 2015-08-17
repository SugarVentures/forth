
#ifndef OPPVS_PUBLISH_CHANNEL
#define OPPVS_PUBLISH_CHANNEL

#include "channel.hpp"


namespace oppvs
{

	class PublishChannel : public Channel
	{
	public:
		PublishChannel(void* owner, callbackNewSubscriber event);
		virtual ~PublishChannel();
		int start();
		void waitingSubscribers();

		static void* run(void* object);
		static void callbackCandidateGatheringDoneImpl(void* object, std::string username, std::string password, std::vector<oppvs::IceCandidate>& candidates);
		static void callbackOnIceResponse(void* object, std::string& username, std::string& password, std::vector<oppvs::IceCandidate>& candidates);
	private:
		ServerSocket m_server;
		bool m_interrupt;
		callbackNewSubscriber m_event;
		void* m_owner;
		Thread* m_thread;

		
	};
}

#endif