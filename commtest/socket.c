#include "serial.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define PORT		30275

#define OUTPUT_BUFFER_SIZE	256

int socket_handle;
unsigned char output_buffer[OUTPUT_BUFFER_SIZE];
int output_buffer_index;

int serial_init() {
	//Declare Variables
	struct sockaddr_in address;
	struct hostent* server;
	fd_set read_flags;

	//Create socket
	socket_handle = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_handle <0) {
		printf("Error creating socket.\n");
		return -1;
	}

	//Make it non-blocking
	if (0 != makenonblocking(socket_handle)) {
		printf("Error unblocking socket.\n");
		return -1;
	}

	//Prepare server address structure
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);
	
	//Connect socket
	if (connect(socket_handle, (struct sockaddr*)&address, sizeof(address)) < 0) {
		//Wait until non-blocking operation finishes
		usleep(100000);
		if (connect(socket_handle, (struct sockaddr*)&address, sizeof(address)) < 0) {
			printf("Unable to connect socket (%s)\n", strerror(errno));
			return -1;
		}
	}

	//Initialise buffer index
	output_buffer_index = 0;
	
	return 0;
}
void serial_uninit() {
	//Close socket if set
	if (socket_handle != 0);
		close(socket_handle);
}

int serial_read(unsigned char* buffer) {
	return read(socket_handle, buffer, 1);
}
int serial_write(unsigned char c) {
	//Declare Variables
	fd_set write_flags;
	
	FD_ZERO(&write_flags);
	FD_SET(socket_handle, &write_flags);

	if (select(socket_handle+1, 0, &write_flags, 0, 0) < 0) {
		//Error waiting for non-blocking write
		return -1;
	}
	if (FD_ISSET(socket_handle, &write_flags)) {
		//Ready to write
		return write(socket_handle, &c, 1);
	} else {
		//?
		return -1;
	}
}

int makenonblocking(int s) {
	//Declare Variables
	int flags;

	//Get flags
	flags = fcntl(s, F_GETFL);
	if (flags < 0) {
		printf("Marco~!\n");
		return -1;
	}
	//Add non-blocking flag and set
	flags |= O_NONBLOCK;
	if (fcntl(s, F_SETFL, flags) < 0) {
		printf("Polo~!\n");
		return -1;
	}
	return 0;
}
