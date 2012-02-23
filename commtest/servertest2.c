#include "comm.h"
#include "convert.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


#include "log.h"

#define LOGFILE_LEN		0xFFFF

unsigned char logfile[LOGFILE_LEN];
int logindex = LOGFILE_LEN;
int logflag;

void log_transmit();

int main() {
	comm_type type;
	unsigned char* data;
	int len;
	int result;
	int I;
	int len1, len2;
	unsigned char* data1;
	unsigned char* data2;
	FILE* fh;
	
	int index;
	unsigned char buffer[CHUNK_SIZE_7BIT];

	if (0 != comm_init())
		return;
		
	srand(time(NULL));
	
	//Create logfile
	for (I = 0; I < LOGFILE_LEN; I++) {
		logfile[I] = rand() % 256;
		//Approximate serial link speed
		usleep(100);
	}
	//Write for check after fact
	fh = fopen("log1.bin", "w");
	fwrite(logfile, 1, LOGFILE_LEN, fh);
	fclose(fh);

/*	
	printf("Logfile:\t");
	printbytes(logfile, 14);
	printf("\n");
	
	//Convert
	len1 = get8to7bitchunklen(LOGFILE_LEN);
	data1 = malloc(len1);
	printf("Converting from %i to %i bytes\n", LOGFILE_LEN, len1);
	convert8to7bitchunk(logfile, LOGFILE_LEN, data1);
	
	printf("Converted:\t");
	printbytes(data1, len1);
	printf("\n");
	
	
	//Convert it back
	len2 = get7to8bitchunklen(len1);
	data2 = malloc(len2);
	printf("Converting from %i to %i bytes\n", len1, len2);
	convert7to8bitchunk(data1, len1, data2);
	
	printf("Reconverted:\t");
	printbytes(data2, len2);
	printf("\n");
	
	//Verify contents
	for (I = 0; I < LOGFILE_LEN; I++) {
		if (data2[I] != logfile[I]) {
			printf("Conversion error on byte %i\n", I);
			break;
		}
	}
	free(data1);
	free(data2);
*/

	while (1) {
		//Receive data
		len = 0;
		result = recv_data(&type, &data, &len);
		
		if (result == 1) {
			printf("Received data (type=%i):\t", type, len);
			printbytes(data, len);
			printf("\n");
			
			//Log request
			if (type == REQ_LOG) {
				//Go handle log sending and stuff
				log_transmit();
			} else if (type == ACK) {
				printf("ACK!\n");
				break;
			}
			
			free(data);
		} else if (result == -1) {
			printf("What a mess!");
			break;
		}
		
		usleep(100);
	}

	comm_uninit();

	return 0;
}

void log_transmit() {
	//Declare Variables
	unsigned char c;
	comm_type type;
	unsigned char* data;
	int len;
	
	unsigned char* intbuffer;
	int intbufferlen;
	
	int index;
	unsigned char buffer[2*CHUNK_SIZE_7BIT];

	//Log a transmit event
	log_event(LOG_TRANSMIT);

	//Stop logging to prevent logfile corruption
	log_event(LOG_STOP);
	logflag = 0;

	//Send logfile size to sendable format
	//NOTE: _swap only while testing, not on qr side
	make_int_sendable_swap(logindex, &intbuffer, &intbufferlen);

	if (0 != send_data(LOG_SIZE, intbuffer, intbufferlen)) {
		//TODO: Handle error
		return;
	}
	
	free(intbuffer);

	//TODO: Make this fancier
	//Send entire logfile in a blocking fashion

	//Wait for data
	while (1) {
		//Receive request
		while (0 == recv_data(&type, &data, &len));
		
		//Check for log chunk request
		if (type != REQ_LOG_CHUNK) {
			//Other side is no longer interested in receiving logfile chunks
			break;
		}

		//Retrieve index from data
		//NOTE: _swap only while testing, not on qr side
		index = make_int_swap(data);

		//Free allocated memory from recv_data
		free(data);

		//Convert specific chunk to communicable format
		if (logindex - index > 2*CHUNK_SIZE_8BIT)
			convert8to7bitchunk(&logfile[index], 2*CHUNK_SIZE_8BIT, buffer);
		else
			convert8to7bitchunk(&logfile[index], logindex - index, buffer);

		//Send the chunk
		if (0 != send_data(LOG_CHUNK, buffer, 2*CHUNK_SIZE_7BIT)) {
			//TODO: Handle error
			break;
		}
	}

	//Reactivate log
	logflag = 1;
	log_event(LOG_START);
}
