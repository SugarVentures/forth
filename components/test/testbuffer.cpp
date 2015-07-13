#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>

#include "dynamic_buffer.hpp"

using namespace oppvs;

void thr(std::shared_ptr<DynamicBuffer> p)
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::shared_ptr<DynamicBuffer> lp = p; // thread-safe, even though the
                                  // shared use_count is incremented
    {
      static std::mutex io_mutex;
      std::lock_guard<std::mutex> lk(io_mutex);
      std::cout << std::this_thread::get_id() << '\n';
      lp->setSize(lp->size() + 2);
      std::cout << "local pointer in a thread:\n"
                << "  lp.size() = " << lp.get()
                << ", lp.use_count() = " << lp.use_count() << " size: " << lp->size() << '\n';
    }
}

int main()
{
	uint8_t data[10];
	DynamicBuffer buffer(data, 10);
	std::cout << buffer.size() << std::endl;
	
	buffer.setCapacity(20);

	std::cout << buffer.size() << " " << buffer.capacity() << std::endl;

	DynamicBuffer buffer2(buffer);
	std::cout << buffer2.size() << " " << buffer2.capacity() << std::endl;

	DynamicBuffer buffer3(data, 5);

	if (buffer == buffer2)
		std::cout << "Equal" << std::endl;

	if (buffer3 != buffer)
	{
		std::cout << "Not equal" << std::endl;
	}

	buffer3 = buffer;

	if (buffer3 == buffer2)
		std::cout << "Equal" << std::endl;

	buffer3.appendData(data, 10);
	std::cout << buffer3.size() << " " << buffer3.capacity() << std::endl;

	buffer3.appendData(data, 10);
	std::cout << buffer3.size() << " " << buffer3.capacity() << std::endl;


	buffer3.moveTo(&buffer);
	std::cout << "Buffer: " << buffer.size() << " " << buffer.capacity() << std::endl;
	std::cout << "Buffer3: " << buffer3.size() << " " << buffer3.capacity() << std::endl;

	std::shared_ptr<DynamicBuffer> p(new DynamicBuffer(data, 10));
	std::thread t1(thr, p), t2(thr, p), t3(thr, p);
    p.reset(); // release ownership from main
    std::cout << "Shared ownership between 3 threads and released\n"
              << "ownership from main:\n"
              << "  p.get() = " << p.get()
              << ", p.use_count() = " << p.use_count() << '\n';
    t1.join(); t2.join(); t3.join();
    


	return 0;
}