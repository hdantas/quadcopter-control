#include "log.h"
#include "log_pc.h"
#include "comm.h"
//#include "convert.h"
#include "serial.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define FILENAME_LOG_RAW	"log.bin"
#define FILENAME_LOG_PARSED "log.txt"

/*	Log the occurrence of an event on the PC-side
 *	Author: Maurijn Neumann
 */
void log_event(log_type event) {
//	printf("Log event: %i\n", event);
}
/*	Log an integer value on the PC-side
 *	Author: Maurijn Neumann
 */
void log_int(int value) {
//	printf("Log int: %i\n", value);
}
/*	Log a byte of data on the PC-side
 *	Author: Maurijn Neumann
 */
void log_byte(unsigned char c) {
//	printf("Log byte: %.02X\n", c);
}
/*	Log one or more bytes of data on the PC-side
 *	Author: Maurijn Neumann
 */
void log_data(log_type type, unsigned char* data, int len) {
	//Declare Variables
/*	int I;
	
	printf("Log data: %i\t", type);
	for (I = 0; I < len; I++)
		printf("%.02X ", data[I]);
	printf("\n");
*/	
}
/*	Log a text message on the PC-side
 *	Author: Maurijn Neumann
 */
void log_msg(const char* msg) {
	printf("Log message: %s\n", msg);
}

/*	Retrieves logfile from QR and stores on disk in binary format
 *	Blocking and ugly
 *	Returns 0 on success, -1 otherwise
 *	Author: Maurijn Neumann
*/
int retrieve_log() {
	//Declare Variables
	comm_type type;
	unsigned char* data;
	int len;
	unsigned int logsize;
	unsigned char* logfile;
	FILE* fh;
	int I;
	int phase;
	unsigned char c;

	//Send logfile request
	if (0 != send_data(REQ_LOG, 0, 0)) {
		printf("Error sending log request\n");
		return -1;
	}
	
	//Wait for ready to send
	while (1) {
		if (1 == recv_data(&type, &data, &len)) {
			//Discard data
			free(data);
			//Process data
			if (type == REQ_LOG)
				//Ready
				break;
			else if (type == UNAVAILABLE)
				//Not ready
				return -1;
		}
	}
	
//	printf("Waiting for logfile size\n");
	
	//Initialize
	phase = 1;
	I = 0;
	logsize = 0;
	
	while (phase < 3) {
		//Wait for character
		while (0 == serial_read(&c));
		
		if (phase == 1) {
			//Process log size
			logsize |= c << (24 - 8 * I);
			I++;
			if (I == 4) {
				//Create logfile
				printf("Log size: %i\n", logsize);
				logfile = malloc(logsize);
				//Proceed
				phase++;
				I = 0;
			}
		} else if (phase == 2) {
			//Add byte to logfile
			logfile[I] = c;
			I++;
			if (I == logsize) {
				//Proceed
				phase++;
			}
		}
	}

	//Logfile read completely
//	printf("Successfully acquired logfile\n");

	//Parse it
//	printf("Parsing logfile...\n");
	parse_log(logfile, logsize);
//	printf("Done parsing logfile.\n");

	//Output raw logfile to file
	fh = fopen(FILENAME_LOG_RAW, "w");
	if (fh == 0) {
		printf("Error opening %s.\n", FILENAME_LOG_RAW);
		return -1;
	}
	if (0 == fwrite(logfile, 1, logsize, fh)) {
		printf("Error writing to file.\n");
		return -1;
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
//	printf("Got here!\n");

	return 0;
}

/*	Translates binary logfile into human-readable format
 *	Author: Maurijn Neumann
 */
void parse_log(unsigned char* logfile, int logsize) {
	//Declare Variables
	int logindex;
	unsigned int timestamp;
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
		timestamp = other_endian(timestamp);
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
				memcpy(&value, &logfile[logindex], 4);
				value = other_endian(value);
//				value = make_int(&logfile[logindex]);
				sprintf(outputstr, "0x%.08X = %i", value, value);
				fputs(outputstr, fh);
				logindex += 4;
			} else if (type == LOG_VALUES) {
				//Loop through 16 stored integers
				for (I = 0; I < 16; I++) {
					//Write integer to file in both decimal and hex representations
					memcpy(&value, &logfile[logindex], 4);
					value = other_endian(value);
	//				value = make_int(&logfile[logindex]);
					sprintf(outputstr, "%i\t", value);
					fputs(outputstr, fh);
					logindex += 4;
				}
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
	
}
/*	Convert the enumerated type of a log entry to string
 *	Author: Maurijn Neumann
 */
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
		case LOG_VALUES:
			return "LOG_VALUES";

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

