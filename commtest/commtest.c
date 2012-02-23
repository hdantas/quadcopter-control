#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "comm.h"
#include "log.h"
#include "serial.h"

#define TEST_SIZE	10000
#define DELAY_US	100

#define SIMPLE_PACKET_PERCENTAGE	60
#define	DATA_PACKET_PERCENTAGE		20	
#define INVALID_CHECKSUM			5
#define INVALID_LENGTH				5
#define FRAME_BUFFER_FULL			1
#define STRAY_BYTE					//100% - rest

int main(void) {
	//Declare variables
	int I, J;
	int testtype;
	comm_type type;
	unsigned char* data;
	int len;

	int result;

	//Initialise communication
	if (0 != comm_init())
		return -1;

	//Initialise random numbers
	srand(time(NULL));

	//Loop through tests
	for (I = 0; I < TEST_SIZE; I++) {
		//Generate a data type at random
		testtype = rand() % 100;

		if ((testtype < SIMPLE_PACKET_PERCENTAGE) && (testtype >= 0)) {
			printf("Simple packet\n");
			send_data(rand() % 5, 0, 0);
		}
		testtype -= SIMPLE_PACKET_PERCENTAGE;

		if ((testtype < DATA_PACKET_PERCENTAGE) && (testtype >= 0)) {
			printf("Data packet\n");
			make_int_sendable(rand(), &data, &len);

			send_data(DEBUG_INT, data, len);
			free(data);
		}
		testtype -= DATA_PACKET_PERCENTAGE;

		if ((testtype < INVALID_CHECKSUM) && (testtype >= 0)) {
			printf("Invalid checksum\n");
			data = malloc(2);
			data[0] = 0x80 | (rand() % 5);		//Head
			data[1] = 0xC0 | (rand() % 0x40);	//Tail
			log_data(LOG_SEND_PACKET, data, 2);
			serial_write(data[0]);
			serial_write(data[1]);
			free(data);
		}
		testtype -= INVALID_CHECKSUM;

		if ((testtype < INVALID_LENGTH) && (testtype >= 0)) {
			printf("Invalid length\n");
			len = rand() % 8;
			data = malloc(len+2);
			if (testtype < INVALID_LENGTH/2)
				data[0] = 0x80 | DEBUG_INT;			//Head
			else
				data[0] = 0x80 | (rand() % 5);		//Head
			for (J = 1; J <= len; J++)
				data[J] = rand() % 0x80;			//Body
			data[len+1] = 0xC0 | (rand() % 0x40);	//Tail
			log_data(LOG_SEND_PACKET, data, len+2);
			for (J = 0; J < len+2; J++)
				serial_write(data[J]);
			free(data);
		}
		testtype -= INVALID_LENGTH;

		if ((testtype < FRAME_BUFFER_FULL) && (testtype >= 0)) {
			printf("Filling frame buffer\n");
			len = FRAME_BUFFER_SIZE + 2;
			data = malloc(len);
			data[0] = 0x80 | (rand() % 5);			//Head
			for (J = 1; J < FRAME_BUFFER_SIZE; J++)
				data[J] = rand() % 0x80;			//Body
			data[len-1] = 0xC0 | (rand() % 0x40);	//Tail
			log_data(LOG_SEND_PACKET, data, len);
			for (J = 0; J < len; J++)
				serial_write(data[J]);
			free(data);
		}
		testtype -= FRAME_BUFFER_FULL;

		if (testtype >= 0) {
			printf("Stray byte\n");
			data = malloc(1);
			data[0] = rand() % 0x0100;
			log_data(LOG_SEND_PACKET, data, 1);
			serial_write(data[0]);
			free(data);
		}

		//Delay
		usleep(DELAY_US);
		
		//See if data is to be received
		result = recv_data(&type, &data, &len);

		while (result == 1) {
			//Handle data? do nothing?

			//Free buffer
			free(data);
			//Check for more data
			result = recv_data(&type, &data, &len);
		}
	}

	//Uninitialise
	comm_uninit();

	return 0;
}
