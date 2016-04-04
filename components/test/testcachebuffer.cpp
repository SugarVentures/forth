#include <stdio.h>

#include "cache_buffer.hpp"
#include "event_handler.hpp"

#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>


using namespace oppvs;

std::mutex m;
std::condition_variable cv;
std::string data;
bool ready = false;
bool processed = false;

void thrAdd(CacheBuffer* p, SharedDynamicBufferRef ref, int i)
{
    std::this_thread::sleep_for(std::chrono::seconds(1));

    CacheBuffer *lp = p; // thread-safe, even though the
                                  // shared use_count is incremented
    {	
    	
    	lp->add(i, ref);
		
    }
}

void thrGet(void* p)
{
	std::this_thread::sleep_for(std::chrono::seconds(10));
	CacheBuffer *lp = (CacheBuffer*)p;
	lp->remove(10);
}

void callbackExample(void* vp)
{
	//std::cout << s << "\n";
	std::string *sp = static_cast<std::string*>(vp);
	// You could use 'sp' directly, or this, which does a copy.
	std::string s = *sp;
	// Don't forget to destroy the memory that you've allocated.
	delete sp;
	std::cout << s << "\n";

}

void thrEvent(EventHandler* handler, int i, void* param)
{
	printf("Event thread\n");

	std::this_thread::sleep_for(std::chrono::seconds(1));
	if (i == 5 || i == 9)
	{
		void *vp = static_cast<void*>(new std::string("Thread "));
    	handler->sendSignal(EVENT_SIGNAL_EXECUTE, [](void* obj) { return callbackExample(obj); }, vp);

	}
	if (i == 2)
	{
		handler->sendSignal(EVENT_SIGNAL_EXECUTE, [](void* obj) { return thrGet(obj); }, param);
	}
}

void worker_thread()
{
    // Wait until main() sends data
    std::unique_lock<std::mutex> lk(m);
    cv.wait(lk, []{return ready;});
 
    // after the wait, we own the lock.
    std::cout << "Worker thread is processing data\n";
    data += " after processing";
 
    // Send data back to main()
    processed = true;
    std::cout << "Worker thread signals data processing completed\n";
 
    // Manual unlocking is done before notifying, to avoid waking up
    // the waiting thread only to block again (see notify_one for details)
    lk.unlock();
    cv.notify_one();
}

void* runThread(void* param)
{
	std::cout<<"Run thread\n";
	return NULL;
}



int main()
{
	EventHandler* handler;
	handler = EventHandler::getInstance();

	SharedDynamicBufferRef bufRef(new DynamicBuffer());

	CacheBuffer cache;
	cache.add(0, bufRef);
	cache.add(3, bufRef);
	cache.add(2, bufRef);
	cache.add(1, bufRef);
	cache.add(2, bufRef);
	cache.add(0, bufRef);
	cache.add(-2, bufRef);

	srand(time(NULL));
	int ts;
	for (int i = 0; i < 100; i++)
	{
		//ts = rand() % 100;
		ts = i;
		cache.add(ts, bufRef);
	}

	//Test multithread
	/*std::thread ta[2];
	for (int i = 0; i < 2; i++)
	{
		ta[i] = std::thread(thrAdd, &cache, bufRef, i);
	}

    
	*/
	printf("Size of map %zu\n", cache.size());
	cache.printBuffer();

	//std::thread worker(worker_thread);
 
    /*data = "Example data";
    // send data to the worker thread
    {
        std::lock_guard<std::mutex> lk(m);
        ready = true;
        std::cout << "main() signals data ready for processing\n";
    }
    cv.notify_one();
 
    // wait for the worker
    {
        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk, []{return processed;});
    }*/
    //std::cout << "Back in main(), data = " << data << '\n';
 
    //worker.join();
    
    //std::cout << "Number of threads = " 
    //          <<  std::thread::hardware_concurrency() << std::endl;

    

    //Test multithread
	/*size_t n = 10;
	std::thread t[10];
	for (int i = 0; i < n; i++)
	{
		t[i] = std::thread(thrEvent, &handler, i, &cache);
	}*/

	//for (auto& th : ta) th.join();
	//for (auto& th : t) th.join();

    std::this_thread::sleep_for(std::chrono::seconds(2));
	handler->stop();
	return 0;
}