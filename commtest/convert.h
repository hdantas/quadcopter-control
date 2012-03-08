#ifndef _CONVERT_H
#define _CONVERT_H

#define CHUNK_SIZE_7BIT		8
#define CHUNK_SIZE_8BIT		7

#define get8to7bitchunklen(len)	((int)((len+CHUNK_SIZE_8BIT-1)/CHUNK_SIZE_8BIT)) * CHUNK_SIZE_7BIT
#define get7to8bitchunklen(len) len * CHUNK_SIZE_8BIT/CHUNK_SIZE_7BIT

void convert8to7bitchunk(unsigned char* in, int inlen, unsigned char* out);
void convert7to8bitchunk(unsigned char* in, int inlen, unsigned char* out);

//int convert8to7bitint(int value);
//int convert7to8bitint(int value);

unsigned int other_endian(unsigned int value);

unsigned int make_int(unsigned char* buffer);
void make_int_sendable(unsigned int value, unsigned char** buffer, int* len);
unsigned int make_int_swap(unsigned char* buffer);
void make_int_sendable_swap(unsigned int value, unsigned char** buffer, int* len);


//TODO: Remove
void printbytes(unsigned char* data, int len);

#endif
