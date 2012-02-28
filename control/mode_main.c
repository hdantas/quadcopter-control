#include <stdio.h>
#include "comm.h"
#include "log.h"

int main(void) {
	int len=1;

	//Initialise communication
	if (0 != comm_init())
		return -1;


	printf("PC sending manual\n");
	if (0 != send_data(KEYQ, 0, 0))
		return -2;
	printf("PC sent manual\n");


	//Uninitialise
	comm_uninit();
	return 0;
}
