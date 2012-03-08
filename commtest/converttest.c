#include "convert.h"
#include <stdio.h>

void printrawbytes(unsigned char* bytes, int len);

void main(void) {
	unsigned char data[CHUNK_SIZE_8BIT];
	unsigned char buffer[CHUNK_SIZE_7BIT];
	int I;

	//Setup
	for (I = 0; I < CHUNK_SIZE_8BIT; I++)
		data[I] = 3*I + 0x70;

	printf("Initially...\n\t");
	printrawbytes(data, CHUNK_SIZE_8BIT);
	printf("\n");

	printf("Converting...\n\t");
	convert8to7bitchunk(data, CHUNK_SIZE_8BIT, buffer);
	printrawbytes(buffer, CHUNK_SIZE_7BIT);
	printf("\n");

	printf("Converting back...\n\t");
	convert7to8bitchunk(buffer, data);
	printrawbytes(data, CHUNK_SIZE_8BIT);
	printf("\n");
	
}

void printrawbytes(unsigned char* bytes, int len) {
	int I;
	for (I = 0; I < len; I++)
		printf("%.02X ", bytes[I]);
}
