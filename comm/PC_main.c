#include <stdio.h>
#include <unistd.h>
#include "log.h"
#include "serial.h"

int main(void) {
	//Declare variables
/*	unsigned char axes[4];*/
/*	comm_type type;*/
/*	int len;*/
/*	int I;*/

	//Initialise communication
	if (0 != serial_init())
		return -1;


	//Send keypress of Q
	printf("PC sending Key: Q\n");
	if (1 != serial_write('Q'))
		return -2;

	
	//Send keypress of W
	printf("PC sending Key: W\n");
	if (1 != serial_write('W'))
		return -3;

	
/*	//Create dummy axes values*/
/*	axes[0] = 0x05;	axes[1] = 0x17;*/
/*	axes[2] = 0x42;	axes[3] = 0x00;*/
/*	//Send*/
/*	printf("PC sending Axes...\n");*/
/*	if (0 != send_data(RPYL, axes, 4))*/
/*		return -4;*/
/*	sleep(1);*/

	//Request logfile
	printf("PC requesting logfile\n");
	log_retrieve();


	//Send keypress of Esc
	printf("PC sending Key: ESC\n");
	if (1 != serial_write('E'))
		return -5;

/*	printf("Waiting for data...\n");
	while (0 == recv_data(&type, &data, &len));
*/
	//Uninitialise
	serial_uninit();
	return 0;
}
