#include "server.h"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <fcntl.h>

#define HOSTNAME	"localhost"
#define PORT		30275

int server_handle;
int socket_handle[2];

int init() {
	//Declare Variables
	struct sockaddr_in address;
	int reuse_addr;

	//Create socket
	server_handle = socket(AF_INET, SOCK_STREAM, 0);
	if (server_handle <0) {
		printf("Error creating socket.\n");
		return -1;
	}

	//Allow rebinding
	reuse_addr = 1;
	setsockopt(server_handle, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));

/*	//Make it non-blocking
	if (0 != makenonblocking(server_handle)) {
		printf("Error unblocking socket.\n");
		return -1;
	}
*/
	//Prepare server address structure
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);
	
	//Bind socket socket
	if (bind(server_handle, (struct sockaddr*)&address, sizeof(address)) < 0) {
		printf("Error binding socket. (%s)\n", strerror(errno));
		uninit();
		return -1;
	}

	//Listen for incoming connections
	listen(server_handle, 2);

	return 0;
}
void uninit() {
	//Close sockets if open
	if (server_handle != 0);
		close(server_handle);
	if (socket_handle[0] != 0);
		close(socket_handle[0]);
	if (socket_handle[1] != 0);
		close(socket_handle[1]);
}

int makenonblocking(int s) {
	//Declare Variables
	int flags;

	//Get flags
	flags = fcntl(s, F_GETFL);
	if (flags < 0)
		return -1;
	//Add non-blocking flag and set
	flags |= O_NONBLOCK;
	if (fcntl(s, F_SETFL, flags) < 0)
		return -1;
	return 0;
}

void main(void) {
	//Declare Variables
	struct sockaddr_in address[2];
	int len;
	fd_set select_set, read_set;
	unsigned char c;
	unsigned char buffer[256];
	int count, I, J;
	int s, ns;
	
	//Initialize sockets
	printf("Initializing server...\n");
	if (0 != init())
		return;

	//Block until accept
	printf("Ready to accept connections...\n");
	len = sizeof(address[0]);
	socket_handle[0] = accept(server_handle, (struct sockaddr*) &address[0], &len);
	if (socket_handle[0] < 0) {
		printf("Error accepting connection.\n");
		uninit();
		return;
	}

	//Block until accept
	printf("Connection acquired, waiting for second party...\n");
	len = sizeof(address[1]);
	socket_handle[1] = accept(server_handle, (struct sockaddr*) &address[1], &len);
	if (socket_handle[1] < 0) {
		printf("Error accepting connection.\n");
		uninit();
		return;
	}

	//Create select set
	FD_ZERO(&select_set);
	FD_SET(socket_handle[0], &select_set);
	FD_SET(socket_handle[1], &select_set);

	//Read and print
	printf("Ready to forward messages...\n");
	printf("\tS1\tS2\tbinary\t\tdir\n");
	while (1) {
		//Wait for data to come in
		read_set = select_set;
		if (select(FD_SETSIZE, &read_set, NULL, NULL, NULL) < 0) {
			printf("Error during select.\n");
			uninit();
			return;
		}
		//Loop through readable sockets
		for (I = 0; I < 2; I++) {
			if (FD_ISSET(socket_handle[I], &read_set)) {
				//Filter data sockets from rest

				//Read socket (large buffer at a time)
				count = read(socket_handle[I], &buffer, 256);
				if (count < 0) {
					printf("Error while reading...\n");
					uninit();
					return;
				} else if (count == 0) {
					printf("End of file for socket %i...\n", I);
					uninit();
					return;
				}

				//Loop through read bytes
				for (J = 0; J < count; J++) {
					//Fetch character from buffer
					c = buffer[J];

					//Display
					if (I == 0)
						printf("\t%.02X\t\t", c);
					else
						printf("\t\t%.02X\t", c);
					printbin(c);

					//Retransmit
					if (1 != write(socket_handle[(I+1)%2], &c, 1)) {
						printf("\nError while writing...\n");
						uninit();
						return;
					}

					if (I == 0)
						printf("\t=>\n");
					else
						printf("\t<=\n");

				}
			}
		}
	}

	//Close sockets
	uninit();
}

void printbin(unsigned char c) {
	//Declare variables
	int I;

	//Loop through bits
	for (I = 0; I < 8; I++)
		printf("%X", ((c >> (7-I)) & 0x01));
}
