/*
	Debug functions
*/

#ifndef OPPVS_MISC_H
#define OPPVS_MISC_H

#include "brg_types.h"

#ifdef __cplusplus
extern "C" 
{
#endif

//#include "srtp/crypto_kernel.h"
//#include "srtp/sha1.h"


void printHashCode(const void* msg, int len);

#ifdef __cplusplus
}  /* end extern "C" */
#endif

#endif