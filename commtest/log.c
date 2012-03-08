#include "log.h"
#include "log_pc.h"
#include "comm.h"
#include "convert.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define FILENAME_LOG_RAW	"log.bin"
#define FILENAME_LOG_PARSED "log.txt"

/*
	These functions were replaced by macros
*/
void log_event(log_type event) {
	printf("Log event: %i\n", event);
}
void log_int(int value) {
	printf("Log int: %i\n", value);
}
void log_byte(unsigned char c) {
	printf("Log byte: %.02X\n", c);
}

void log_data(log_type type, unsigned char* data, int len) {
	//Declare Variables
	int I;
	
	printf("Log data: %i\t", type);
	for (I = 0; I < len; I++)
		printf("%.02X ", data[I]);
	printf("\n");
	

}
void log_msg(const char* msg) {
	printf("Log message: %s\n", msg);
}

/*
	Retrieves logfile from QR and stores on disk in binary format
	Blocking and ugly
*/
void retrieve_log() {
	//Declare Variables
	int result;
	comm_type type;
	unsigned char* data;
	int len;
	
	int logsize;
	unsigned char* logfile;
	int logindex;
	
	unsigned char* intbuffer;
	int intbufferlen;
	
	unsigned char buffer[CHUNK_SIZE_8BIT];
	
	FILE* fh;
	int I;
	

	//Send logfile request
	if (0 != send_data(REQ_LOG, 0, 0)) {
		printf("Error sending log request\n");
		return;
	}
	
	printf("Waiting for logfile size\n");

	//Wait for logfile size
	while (0 == recv_data(&type, &data, &len));
	
	printf("Received logfile size\n");

	if (type != LOG_SIZE) {
		printf("Invalid response to logfile request: %i\n", type);
		free(data);
		return;
	}

	//Create logfile
	logsize = make_int_swap(data);
	free(data);

	printf("Log size: %i\n", logsize);
	
	logfile = malloc(logsize);
	
	

	//Retrieve chunk after chunk
	for (logindex = 0; logindex < logsize; logindex += 2*CHUNK_SIZE_8BIT) {
		printf("Requesting chunk %i...\n", logindex);
		
		//Prepare integer
		make_int_sendable_swap(logindex, &intbuffer, &intbufferlen);
		
		//Request chunk
		if (0 != send_data(REQ_LOG_CHUNK, intbuffer, intbufferlen)) {
			printf("Error sending log chunk request\n");
			return;
		}
		//Free memory from make_sendable_int_swap 
		free(intbuffer);

		//Block and wait
		while (1) {
			result = recv_data(&type, &data, &len);
			if (result == 1) {

				if (type == DEBUG_INT) {
					printf("DEBUG_INT\n");
		
				} else if (type != LOG_CHUNK) {
					printf("Invalid response to logfile chunk request: %i\n", type);
					free(data);
					return;
				} else {
					break;
				}
			} else if (result == -1) {
				printf("ERROR!\n");
				return;
			}
		}

		//Convert
		convert7to8bitchunk(data, len, buffer);
		free(data);
	
		//Copy to logfile structure
		if (logsize-logindex < 2*CHUNK_SIZE_8BIT) {
			memcpy(&logfile[logindex], buffer, logsize-logindex);
			printf("Done!\n");
			break;
		} else {
			memcpy(&logfile[logindex], buffer, 2*CHUNK_SIZE_8BIT);
		}
	}

	//Logfile read completely
	printf("Successfully acquired logfile\n");

	//Parse it
//	parse_log(logfile, logsize);


	//Output raw logfile to file
	fh = fopen(FILENAME_LOG_RAW, "w");
	if (fh == 0) {
		printf("Error opening %s.\n", FILENAME_LOG_RAW);
		return;
	}
	//TODO:Limit write chunk size
	if (0 == fwrite(logfile, 1, logsize, fh)) {
		printf("Error writing to file.\n");
		return;
	}
	fclose(fh);
/*	
	//Print logfile
	printf("Logfile:");
	for (I = 0; I < logsize; I++) {
		if (I % 16 == 0)
			printf("\n%.04X:\t", I);
		printf("%.02X ", logfile[I]);
	}
	printf("\n\n");
*/
	//Free memory
	free(logfile);
}

/*
	Translates binary logfile into human-readable format
*/
void parse_log(unsigned char* logfile, int logsize) {
	//Declare Variables
	int logindex;
	int timestamp;
	unsigned char type;
	int len;
	int I;
	FILE* fh;
	const char* str;
	unsigned char outputstr[512];
	int value;

	

	//Open file for parsed log
	fh = fopen(FILENAME_LOG_PARSED, "w");
	if (fh == 0) {
		printf("Error opening %s.\n", FILENAME_LOG_PARSED);
		return;
	}

	//Loop through logfile
	logindex = 0;
	while (logindex < logsize) {
		//Fetch timestamp
		memcpy(&timestamp, &logfile[logindex], 4);
		timestamp = make_int(&logfile[logindex]);
		logindex += 4;
		//Fetch type
		type = logfile[logindex];
		logindex += 1;

		//Write timestamp & type
		str = logtype_to_string(type);
		if (str != 0)
			sprintf(outputstr, "\n%.08X\t%s\t", timestamp, str);
		else
			sprintf(outputstr, "\n%.08X\t%i\t", timestamp, type);
		fputs(outputstr, fh);
		
		//Evaluate type
		if (type >= LOG_TYPE_ERROR) {
			//Error
			printf("Error %i", type);
			
		} else if (type >= LOG_TYPE_DATA_VAR) {
			//Fetch data length
			len = logfile[logindex];
			logindex += 1;

			if (type == LOG_MSG) {
				//Data is text, output directly
				fwrite(&logfile[logindex], 1, len, fh);
			} else {
				//Write data in hex format
				for (I = 0; I < len; I++) {
					sprintf(outputstr, "%.02X ", logfile[logindex+I]);
					fputs(outputstr, fh);
				}
			}
			logindex += len;

		} else if (type >= LOG_TYPE_DATA_STATIC) {
			//Data types with static length
			if (type == LOG_BYTE) {
				//Write byte to file in hex representation
				sprintf(outputstr, "0x%.02X", logfile[logindex]);
				fputs(outputstr, fh);
				logindex += 1;
			} else if (type == LOG_INT) {
				//Write integer to file in both decimal and hex representations
				value = make_int(&logfile[logindex]);
				sprintf(outputstr, "0x%.08X = %i", value, value);
				fputs(outputstr, fh);
				logindex += 4;
			}

		} else if (type >= LOG_TYPE_EVENT) {
			//No data, just an event
			//printf("Event %i", type);
		}
	}

	//Finish file
	fputs("\n", fh);

	//Close file
	fclose(fh);
	
	//Send and acknowledgement
	if (0 != send_data(ACK, 0, 0))
		//TODO: 
		return;
}

const char* logtype_to_string(log_type type) {
	switch (type) {
		//Events
		case LOG_START:
			return "LOG_START";
		case LOG_STOP:
			return "LOG_STOP";
		case LOG_TRANSMIT:
			return "LOG_TRANSMIT";

		case LOG_BYTE:
			return "LOG_BYTE";
		case LOG_INT:
			return "LOG_INT";

		//Events with data
		case LOG_RECV_PACKET:
			return "LOG_RECV_PACKET";
		case LOG_SEND_PACKET:
			return "LOG_SEND_PACKET";

		case LOG_COMM_DISCARD_BUFFER:
			return "LOG_COMM_DISCARD_BUFFER";

		case LOG_MSG:
			return "LOG_MSG";

		default:
			return 0;
	}
}

