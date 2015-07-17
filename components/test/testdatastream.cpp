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
      uint8_t no = 1;
      if (i % 2)
      	lp->write(&no, 1);
      else
      {
      	lp->read(&no, 1);
      	
      }
      
      static std::mutex io_mutex;
      std::lock_guard<std::mutex> lk(io_mutex);
      
      std::cout << "Size of data stream: " << std::this_thread::get_id() << " " << lp->size() << std::endl;
    }
}

int main()
{
	DataStream ds;
	ds.grow(2);
	std::cout << ds.size() << " " << ds.capacity() << std::endl;

	SharedDynamicBufferRef bufRef(new DynamicBuffer());
	DataStream dataStream(bufRef);


	uint16_t a = 12;
	dataStream.write(&a, sizeof(uint16_t));
	dataStream.grow(3);
	std::cout << "Size of data stream: " << dataStream.size() << " " << dataStream.capacity() << std::endl;

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
	size_t n = 10;
	std::thread t[10];
	for (int i = 0; i < n; i++)
	{
		t[i] = std::thread(thr, &dataStream, i);

	}
	
    for (auto& th : t) th.join();
    

	return 0;
}