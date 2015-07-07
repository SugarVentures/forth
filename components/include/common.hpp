#ifndef OPPVS_COMMON_HPP
#define OPPVS_COMMON_HPP

#include <assert.h>
#include <iostream>

#define DEBUG 1


#if( DEBUG == 1 )
#define ASSERT( expression ) 				        \
  	oppvs::localAssert(expression, __FUNCTION__, __FILE__, __LINE__, #expression)
	
#elif( DEBUG == 0 )
#define ASSERT( result )					\
	void(0)
#endif 

namespace oppvs 
{
	bool localAssert(bool result, const char* function, const char* file, int line, const char* expression)
	{
		if (!result)
		{
			std::cout << expression;
			std::cout << " Failed in file: " << file ; 
			std::cout << " at function: " << function;
  			std::cout << " at line: " << line << std::endl;
		}
		return result;
	}
}

#endif