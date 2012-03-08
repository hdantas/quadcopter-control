#include "convert.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void printbytes(void* p);
//void printbin(unsigned char c);

/*
	in		start of input
	len		number of bytes to put in chunk
	out		output buffer, should be predefined with length multiple of CHUNK_LEN_7BIT
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

	if (len % CHUNK_SIZE_8BIT > 0) {
		//Calculate missing bytes for complete chunk
		I = CHUNK_SIZE_8BIT - (len % CHUNK_SIZE_8BIT);
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
void convert7to8bitchunk(unsigned char* in, int len, unsigned char* out) {
	//Declare variables
	int I, J;
	int K;
	unsigned char remainder;
	
	//Verify length
	if (len % CHUNK_SIZE_7BIT > 0) 
		//TODO: Throw error or smth?
		return;

	//Initialize
	K = get7to8bitchunklen(len) - 1;
	remainder = 0;

	//Loop through bytes from right to left
	for (I = len-1; I >= 0; I--) {
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
	Make an integer out of a CHUNK_SIZE_7BIT char array;
		change endianness for _swap versions
*/
unsigned int make_int(unsigned char* buffer) {
	//Declare Variables
	unsigned char data[CHUNK_SIZE_8BIT];
	unsigned int value;
	//Convert from sendable format
	convert7to8bitchunk(buffer, CHUNK_SIZE_7BIT, data);
	//Copy int variable
	memcpy(&value, &data, 4);
	//Change endianness and return
	return value;
}
void make_int_sendable(unsigned int value, unsigned char** buffer, int* len) {
	//Declare Variables
	unsigned char intbuffer[4];
	
	//Create buffer
	*len = get8to7bitchunklen(4);
	*buffer = malloc(*len);
	memset(*buffer, 0, *len);
	
	//Convert to sendable format
	memcpy(intbuffer, &value, 4);
	convert8to7bitchunk(intbuffer, 4, *buffer);
}
unsigned int make_int_swap(unsigned char* buffer) {
	//Declare Variables
	unsigned char data[CHUNK_SIZE_8BIT];
	unsigned int value;
	//Convert from sendable format
	convert7to8bitchunk(buffer, CHUNK_SIZE_7BIT, data);
	//Copy int variable
	memcpy(&value, &data, 4);
	//Change endianness and return
	return (value >> 24) |
			((value >> 8) & 0x0000FF00) |
			((value << 8) & 0x00FF0000) |
			(value << 24);
}
void make_int_sendable_swap(unsigned int value, unsigned char** buffer, int* len) {
	//Declare Variables
	unsigned char intbuffer[4];
	
	//Create buffer
	*len = get8to7bitchunklen(4);
	*buffer = malloc(*len);
	memset(*buffer, 0, *len);
	
	//Change endianness
	value = (value >> 24) |
			((value >> 8) & 0x0000FF00) |
			((value << 8) & 0x00FF0000) |
			(value << 24);
	
	//Convert to sendable format
	memcpy(intbuffer, &value, 4);
	convert8to7bitchunk(intbuffer, 4, *buffer);
}


/*
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
*/
void printbytes(unsigned char* data, int len) {
	int I;
	for (I = 0; I < len; I++)
		printf("%.02X ", data[I]);
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
