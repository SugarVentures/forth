#include "utility.h"
#include <stdio.h>

void printHashCode(const void* msg, int len)
{
	sha1_ctx_t ctx;
	uint32_t hashcode[5];
	sha1_init(&ctx);

	sha1_update(&ctx, (uint8_t*)msg, len);
	sha1_final(&ctx, hashcode);
		
	printf("len: computed hash value:  %d, %u %u %u %u\n", len, 
		 hashcode[0], hashcode[1], hashcode[2], hashcode[3]);
}