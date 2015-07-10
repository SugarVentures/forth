#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>

#include <stdio.h>
#include "data_stream.hpp"

using namespace oppvs;

void thr(DataStream* p, int i)
{
    std::this_thread::sleep_for(std::chrono::seconds(1));

    DataStream *lp = p; // thread-safe, even though the
                                  // shared use_count is incremented
    {
      //static std::mutex io_mutex;
      //std::lock_guard<std::mutex> lk(io_mutex);
      //std::cout << std::this_thread::get_id() << '\n';
      uint8_t no = 1;
      if (i % 2)
      	lp->write(&no, 1);
      else
      {
      	lp->read(&no, 1);
      	
      }
      
      	std::cout << "Size of data stream: " << std::this_thread::get_id() << " " << lp->size() << std::endl;
    }
}

int main()
{
	SharedDynamicBufferRef bufRef(new DynamicBuffer());
	DataStream dataStream(bufRef);

	uint16_t a = 12;
	dataStream.write(&a, sizeof(uint16_t));
	std::cout << "Size of data stream: " << dataStream.size() << std::endl;

	uint8_t b = 1;
	if (dataStream.setAbsolutePosition(1) < 0)
		return -1;
	dataStream.write(&b, 1);

	if (dataStream.setRelativePosition(-1) < 0)
	{
		std::cout << "setRelativePosition failed " << std::endl;
		return -1;
	}
	dataStream.write(&b, 1);
	std::cout << "Size of data stream: " << dataStream.size() << std::endl;

	dataStream.setAbsolutePosition(1);
	dataStream.read(&b, 1);
	printf("b: %d\n", b);

	std::cout << "multithread" << std::endl;

	//Test multithread
	/*std::thread t[10];
	for (int i = 0; i < 10; i++)
	{
		t[i] = std::thread(thr, &dataStream, i);
		std::cout << "Size of data stream: " << t[i].get_id() << std::endl;
		t[i].join();
	}*/
	std::thread t1(thr, &dataStream, 1);
	t1.join();
	std::thread t2(thr, &dataStream, 2);
	t2.join();
	
    //for (auto& th : t) th.join();
    

	return 0;
}