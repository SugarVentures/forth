#ifndef OPPVS_COMMON_HPP
#define OPPVS_COMMON_HPP

#include <assert.h>
#include <iostream>

#define DEBUG 1


#if( DEBUG == 1 )
#define ASSERT( expression ) 				        \
  	oppvs::localAssert(expression, __FUNCTION__, __FILE__, __LINE__, #expression)
	
#else
#define ASSERT( result )					\
	void(0)
#endif

inline void cta_noop(const char* psz)
{
    ;
}

#define COMPILE_TIME_ASSERT(x) {char name$$[(x)?1:-1]; cta_noop(name$$);}

#ifndef UNREFERENCED_VARIABLE
#define UNREFERENCED_VARIABLE(unrefparam) ((void)unrefparam)
#endif

namespace oppvs 
{
	inline bool localAssert(bool result, const char* function, const char* file, int line, const char* expression)
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