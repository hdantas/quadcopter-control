#include "serial.h"
#include <unistd.h>
#include <stdio.h>

int main() {
	int I;
	unsigned char c;

	serial_init();

	for (I = 0; I < 20; I++) {
		printf("wrote:\t%.02X\t(%i)\n", I, serial_write(I));
		usleep(250000);

		printf("read:\t%.02X\t(%i)\n", c, serial_read(&c));
		usleep(250000);
	}

	usleep(500000);

	serial_uninit();

	return 0;
}
