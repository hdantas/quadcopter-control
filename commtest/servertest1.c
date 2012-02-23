#include "comm.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "log_pc.h"

int main() {
	comm_type type;
	unsigned char* data;
	int len;
	int result;
	int I;

	if (0 != comm_init())
		return;
	
	//Retrieve the log
	printf("Start retrieving the log...\n");
	retrieve_log();
	printf("Done retrieving the log.\n");
	
	send_data(ACK, 0, 0);
	send_data(ACK, 0, 0);

	comm_uninit();

	return 0;
}
