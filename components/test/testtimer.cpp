#include "timer.h"
#include <iostream>

#include <thread>

using namespace oppvs;

int main()
{
	std::cout << getCurrentTime() << "\n";

	Timer<int> timer;

	std::this_thread::sleep_for(std::chrono::seconds(2));
	std::cout<< "diff: " << timer.diff() << "\n";
	return 0;
}