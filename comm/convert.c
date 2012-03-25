#include "convert.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void printbytes(void* p);
//void printbin(unsigned char c);

/*
	in		start of input
	len		number of bytes to put in chunk (<=CHUNK_SIZE_8BIT)
	out		output buffer, should be CHUNK_SIZE_7BIT bytes
*/
void convert8to7bitchunk(unsigned char* in, int len, unsigned char* out) {
	//Declare variables
	int I, J;
	int K;
	unsigned char remainder;

	//Initialize
	K = 0;
	remainder = 0;

	//Loop through bytes
	for (I = 0; I < len; I++) {
		//Mask and copy from input to output
		out[K] = in[I] & 0x7F;
		K++;
		//Mask remaining bit and shift right
		remainder |= in[I] & 0x80;
		remainder >>= 1;

		//Add remainder at 7 byte intervals
		if (I % 7 == 6) {
			//Add remainder to output buffer
			out[K] = remainder;
			K++;
		}
	}

	if (len < CHUNK_SIZE_8BIT) {
		//Calculate missing bytes for complete chunk
		I = (CHUNK_SIZE_8BIT - len) % 7;
		//Shift remainder in proper position
		remainder >>= I;
		//Add to proper place in output buffer
		out[K+I] = remainder;
	}
}

/*
	in		input buffer, should be CHUNK_SIZE_7BIT bytes
	out		output buffer, should be CHUNK_SIZE_8BIT bytes
*/
void convert7to8bitchunk(unsigned char* in, unsigned char* out) {
	//Declare variables
	int I, J;
	int K;
	unsigned char remainder;

	//Initialize
	K = CHUNK_SIZE_8BIT-1;
	remainder = 0;

	//Loop through bytes from right to left
	for (I = CHUNK_SIZE_7BIT-1; I >= 0; I--) {
		//Initialize remainder
		if (I % 8 == 7) {
			remainder = in[I];
			I--;
		}

		//Prepare remainder
		remainder <<= 1;

		//Mask and copy from input to output; append remainder
		out[K] = (remainder & 0x80) | (in[I] & 0x7F);
		K--;
	}
}

/*
	Change little endianness to big endianness and vice versa
*/
unsigned int other_endian(unsigned int value) {
	return (value >> 24) |
		((value >> 8) & 0x0000FF00) |
		((value << 8) & 0x00FF0000) |
		(value << 24);
}
/*
	Make an integer out of a 4byte char array and change endianness
*/
unsigned int make_int(unsigned char* buffer) {
	//Declare Variables
	unsigned int value;
	//Copy buffer to int variable
	memcpy(&value, buffer, 4);
	//Change endianness and return
	return (value >> 24) |
			((value >> 8) & 0x0000FF00) |
			((value << 8) & 0x00FF0000) |
			(value << 24);
}

//Nasty way to compress most of an integer, losing 4 MSB
int convert8to7bitint(int value) {
	return ((value & 0x0FE00000) << 3) |
			((value & 0x001FC000) << 2) |
			((value & 0x00003F80) << 1) |
			(value & 0x0000007F);
}
int convert7to8bitint(int value) {
	return ((value & 0x7F000000) >> 3) |
			((value & 0x007F0000) >> 2) |
			((value & 0x00007F00) >> 1) |
			(value & 0x0000007F);
}

void printbytes(void* p) {
	int I;
	unsigned char c;
	for (I = 0; I < 4; I++) {
		c = *((unsigned char*)p+I);
		printf("%.02X ", c);
	}
}
/*
void printbin(int c) {
	int I;
	for (I = 0; I < 32; I++) {
		if (I % 8 == 0)
			printf(" ");
		if (c & 0x80000000)
			printf("1");
		else
			printf("0");
		c <<= 1;
	}
}
*/
