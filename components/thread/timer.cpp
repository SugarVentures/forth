#include "timer.h"
#include <sstream>

namespace oppvs {

	std::string getCurrentTime()
	{
		time_t currentTime;
		struct tm* localTime;

		time(&currentTime);
		localTime = localtime(&currentTime);

		std::stringstream out;
		out << "Current time: " << localTime->tm_mday << ":" << localTime->tm_mon + 1
			<< ":" << localTime->tm_year + 1900 << " " << localTime->tm_hour
			<< ":" << localTime->tm_min << ":" << localTime->tm_sec;

		return out.str();
	}
} // oppvs