#include "checksum.h"

//#include <stdlib.h>
//#include <string.h>
#include <stdio.h>

unsigned char crc_lookup_table[256];

/*	Calculate a 6-bit CRC using the polynomial x^6 + x + 1
 *	with help of a 256byte lookup table.
 *	Author: Maurijn Neumann
 */
unsigned char calc_checksum(unsigned char* data, int len) {
	//Declare variables
	unsigned char remainder;
	int I;

	//Initialize
	remainder = 0;

	//Loop through data
	for (I = 0; I < len-1; I++) {
		//Shift in new byte and xor
		remainder ^= data[I];

		//Lookup in table
		remainder = crc_lookup_table[remainder];
	}

	//Assume last byte is a tail frame, as such: two set bits plus checksum space (zeroed or present)
	remainder ^= data[len-1];
	remainder = (remainder << 1) ^ CRC_POLYNOMIAL_MSB;	
	remainder = (remainder << 1) ^ CRC_POLYNOMIAL_MSB;

	//Return CRC value
	return (remainder >> 2);
}

/*	Verify a datastream with trailing 6-bit CRC checksum
 *	Returns 1 if checksum is valid, 0 otherwise
 *	Author: Maurijn Neumann
 */
int verify_checksum(unsigned char* data, int len) {
	//Declare variables
	unsigned char checksum;

	//Assume checksum is appended at end, calculate
	checksum = calc_checksum(data, len);

	//Verify
	return (checksum == 0x00);
}
/*	Initialise the CRC lookup table
 *	Author: Maurijn Neumann
 */
void crc_init() {
	//Declare variables
	int data;
	int remainder;
	int J;

	//Create lookup table by iterating over all possible data bytes
	for (data = 0; data < 0xFF; data++) {
		//Initialize remainder
		remainder = data;
		//Execute inner CRC loop
		for (J = 0; J < 8; J++) {
			//Check MSB, highest polynomial coefficient
			if (remainder & 0x80)
				//Shift MSB out, xor with polynomial
				remainder = (remainder << 1) ^ CRC_POLYNOMIAL_MSB;
			else
				//Shift MSB out
				remainder = (remainder << 1);
		}
		//Store result in lookup table
		crc_lookup_table[data] = remainder;
	}
}

/*	Debug function to quickly print a byte in binary format
 *	Author: Maurijn Neumann
 */
void printbin(unsigned char c) {
	//Declare variables
	int I;

	//Loop through bits
	for (I = 0; I < 8; I++)
		printf("%X", ((c >> (7-I)) & 0x01));
}
/*	Previous (less efficient) CRC versions as well as testing code
 *	Author: Maurijn Neumann
/*
void main() {
	int I;
	unsigned char packet[18] = {0xBC, 0x69, 0x6E, 0x67, 0x6C, 0x65,
								0x20, 0x66, 0x00, 0x72, 0x61, 0x6D,
								0x65, 0x00, 0x00, 0x0E, 0x00, 0xC1};

	if (verify_checksum(packet, 18))
		printf("Checksum verified\n");
	else
		printf("Checksum invalid\n");

	packet[17] = 0xC0;
	printf("Recalculation yields: %.02X\n", calc_checksum(packet, 18));
}
*/

/*
unsigned char crc_slow(unsigned char* data, int len) {
	//Declare variables
	unsigned char remainder;
	int I;
	unsigned char next_bit;

	//First 6 bits into remainder
	remainder = data[0] >> 2;

	//Loop through bits
	for (I = 6; I < len*8; I++) {
		//Shift next bit into remainder
		next_bit = (data[I/8] >> (7-I%8)) & 0x01;
		remainder = (remainder << 1) ^ next_bit;
		//XOR if MSB is 1
		if (remainder & 0x40) {
			remainder ^= CRC_POLYNOMIAL_SLOW;
		}
	}

	return remainder;
}

unsigned char crc_medium(unsigned char* data, int len) {
	//Declare variables
	unsigned char remainder;
	int I;
	int J;

	//Initialize
	remainder = 0;

	//Loop through data
	for (I = 0; I < len-1; I++) {
		//Shift in new byte and xor
		remainder ^= data[I];

		//Handle entire byte
		for (J = 0; J < 8; J++) {
			//Check MSB, highest polynomial coefficient
			if (remainder & 0x80)
				//Shift MSB out, xor with polynomial
				remainder = (remainder << 1) ^ CRC_POLYNOMIAL_MSB;
			else
				//Shift MSB out
				remainder = (remainder << 1);
		}
	}

	//Handle last byte separately since last 6 bits are CRC-reserved space
	remainder ^= data[len-1];
	for (J = 0; J < 2; J++) {
		//Check MSB, highest polynomial coefficient
		if (remainder & 0x80)
			//Shift MSB out, xor with polynomial
			remainder = (remainder << 1) ^ CRC_POLYNOMIAL_MSB;
		else
			//Shift MSB out
			remainder = (remainder << 1);
	}

	//Return CRC value
	return (remainder >> 2);
}

void crc_init() {
	//Declare variables
	int data;
	int remainder;
	int J;

	//Create lookup table by iterating over all possible data bytes
	for (data = 0; data < 0xFF; data++) {
		//Initialize remainder
		remainder = data;
		//Execute inner CRC loop
		for (J = 0; J < 8; J++) {
			//Check MSB, highest polynomial coefficient
			if (remainder & 0x80)
				//Shift MSB out, xor with polynomial
				remainder = (remainder << 1) ^ CRC_POLYNOMIAL_MSB;
			else
				//Shift MSB out
				remainder = (remainder << 1);
		}
		//Store result in lookup table
		crc_lookup_table[data] = remainder;
	}
}

unsigned char crc_fast(unsigned char* data, int len) {
	//Declare variables
	unsigned char remainder;
	int I;

	//Initialize
	remainder = 0;

	//Loop through data
	for (I = 0; I < len-1; I++) {
		//Shift in new byte and xor
		remainder ^= data[I];

		//Lookup in table
		remainder = crc_lookup_table[remainder];
	}

	//Assume last byte is a tail frame, as such: two set bits plus checksum space (zeroed or present)
	remainder ^= data[len-1];
	remainder = (remainder << 1) ^ CRC_POLYNOMIAL_MSB;	
	remainder = (remainder << 1) ^ CRC_POLYNOMIAL_MSB;

	//Return CRC value
	return (remainder >> 2);
}


unsigned char crc_fast_lsb(unsigned char* data, int len) {
	//Declare variables
	unsigned char remainder;
	int I;
	int J;

	//Initialize
	remainder = 0;

	//Loop through data
	for (I = 0; I < len; I++) {
		//Shift in new byte and xor
		remainder ^= data[I];
		//Handle entire byte
		for (J = 0; J < 8; J++) {
			//Check LSB, highest polynomial coefficient
			if (remainder & 0x01)
				//Shift LSB out, xor with polynomial
				remainder = (remainder >> 1) ^ CRC_POLYNOMIAL_LSB;
			else
				//Shift LSB out
				remainder = (remainder >> 1);

			printf("remainder:\t");
			printbin(remainder);
			printf("\n");
		}
	}
	
	//Return CRC value
	return remainder;
}
*/
