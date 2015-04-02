/* 
	Test size of data types
*/

#include <stdio.h>
#include "../include/datatypes.hpp"

int main()
{
	printf("Size of uint8_t %lu \n", sizeof(uint8_t));
	printf("Size of uint16_t %lu \n", sizeof(uint16_t));
	printf("Size of uint32_t %lu \n", sizeof(uint32_t));
	printf("Size of uint64_t %lu \n", sizeof(uint64_t));
	printf("Size of int8_t %lu \n", sizeof(int8_t));
	printf("Size of int16_t %lu \n", sizeof(int16_t));
}