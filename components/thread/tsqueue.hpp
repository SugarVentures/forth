#ifndef OPPVS_THREAD_SAFE_QUEUE_HPP
#define OPPVS_THREAD_SAFE_QUEUE_HPP

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <stdlib.h>

namespace oppvs
{
	template <class T>
	class tsqueue
	{
		private: 
			std::queue<T> mData;
		    mutable std::mutex mMut; 
		    std::condition_variable mEmptyCondition;
		    void wait_for_debugging()
		    {
		        //std::this_thread::sleep_for(std::chrono::milliseconds(200+rand()%200));
		    }
		public:
		    void push(T pValue)
		    {
		        std::lock_guard<std::mutex> lock(mMut); 
		        mData.push(pValue); 
		        mEmptyCondition.notify_one(); 
		        wait_for_debugging();
		    }

		    std::shared_ptr<T> try_front()
		    {
		    	std::lock_guard<std::mutex> lock(mMut); 
		        if(mData.empty())
		            return std::shared_ptr<T>(); // null
		        std::shared_ptr<T> ret=std::shared_ptr<T>(std::make_shared<T>(mData.front()));
		        wait_for_debugging();
		        return ret;
		    }

		    std::shared_ptr<T> pop()
		    {
		        std::unique_lock<std::mutex> lock(mMut); 
		        mEmptyCondition.wait(lock, [this]{ return !mData.empty();});
		        std::shared_ptr<T> ret=std::shared_ptr<T>(std::make_shared<T>(mData.front()));
		        mData.pop();
		        wait_for_debugging();
		        return ret;
		    }
		    std::shared_ptr<T> try_pop()
		    {
		        std::lock_guard<std::mutex> lock(mMut); 
		        if(mData.empty())
		            return std::shared_ptr<T>(); // null
		        std::shared_ptr<T> ret=std::shared_ptr<T>(std::make_shared<T>(mData.front()));
		        mData.pop();
		        wait_for_debugging();
		        return ret;
		    }
		    bool empty() const
		    {
		        std::lock_guard<std::mutex> lock(mMut); 
		        return mData.empty();
		    }

		    size_t size() const
		    {
		    	std::lock_guard<std::mutex> lock(mMut); 
		        return mData.size();	
		    }
	};
};
 


#endif