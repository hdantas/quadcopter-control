#include <stdio.h>
#include "comm.h"
#include "log.h"

void main(void) {
	//Declare variables
	unsigned char axes[4];
	comm_type type;
	unsigned char* data;
	int len;
	int I;

	//Initialise communication
	if (0 != comm_init())
		return -1;

	//Send keypress of Q
	printf("PC sending Key: Q\n");
	if (0 != send_data(KEYQ, 0, 0))
		return -2;

	//Send keypress of W
	printf("PC sending Key: W\n");
	if (0 != send_data(KEYW, 0, 0))
		return -3;

	//Create dummy axes values
	axes[0] = 0x05;	axes[1] = 0x17;
	axes[2] = 0x42;	axes[3] = 0x00;
	//Send
	printf("PC sending Axes...\n");
	if (0 != send_data(RPYL, axes, 4))
		return -4;

	//Request logfile
	retrieve_log();

	//Send keypress of Esc
	printf("PC sending Key: ESC\n");
	if (0 != send_data(KEYESC, 0, 0))
		return -5;

/*	printf("Waiting for data...\n");
	while (0 == recv_data(&type, &data, &len));
*/
	//Uninitialise
	comm_uninit();
	return 0;
}
