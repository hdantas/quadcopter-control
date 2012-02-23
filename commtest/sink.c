#include <stdio.h>

#include "comm.h"

int main() {
	//Declare variables
	int result;

	comm_type type;
	unsigned char* data;
	int len;

	//Initialise communication
	if (0 != comm_init())
		return -1;
	
	printf("Ready!\n");

	while (1) {
		//Try for data
		result = recv_data(&type, &data, &len);
		
		if (result == 1) {
			
			//Send it back?
			//send_data(type, data, len);
		}
	}

	//Uninitialise
	comm_uninit();

	return 0;
}
