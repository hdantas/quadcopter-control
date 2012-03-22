#ifndef _CONVERT_H
#define _CONVERT_H

#define CHUNK_SIZE_7BIT		16
#define CHUNK_SIZE_8BIT		CHUNK_SIZE_7BIT * 7/8

void convert8to7bitchunk(unsigned char* in, int len, unsigned char* out);
void convert7to8bitchunk(unsigned char* in, unsigned char* out);

int convert8to7bitint(int value);
int convert7to8bitint(int value);

unsigned int other_endian(unsigned int value);
unsigned int make_int(unsigned char* buffer);

#endif
