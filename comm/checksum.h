#ifndef _CHECKSUM_H
#define _CHECKSUM_H

/*	Author: Maurijn Neumann
 */

unsigned char calc_checksum(unsigned char* data, int len);
int verify_checksum(unsigned char* data, int len);

//x^6 + x + 1     0100 0011
//#define CRC_POLYNOMIAL_SLOW	0x43	//    0100 0011
#define CRC_POLYNOMIAL_MSB	0x0C	//(1) 0000 1100
//#define CRC_POLYNOMIAL_LSB	0x30	//    0011 0000 (1)

void crc_init();

//unsigned char crc_slow(unsigned char* data, int len);
//unsigned char crc_medium(unsigned char* data, int len);
//unsigned char crc_fast(unsigned char* data, int len);
//unsigned char crc_medium_lsb(unsigned char* data, int len);

void printbin(unsigned char c);

#endif
