#include <iostream>

#include "dynamic_buffer.hpp"

using namespace oppvs;

int main()
{
	uint8_t data[10];
	DynamicBuffer buffer(data, 10);
	std::cout << buffer.size() << std::endl;
	
	return 0;
}