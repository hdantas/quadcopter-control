#include "log.h"
#include "x32_log.h"
#include "x32_common.h"
#include "convert.h"
#include "comm.h"

#include <stdlib.h>
#include <string.h>

#define X32_ms_clock		peripherals[PERIPHERAL_MS_CLOCK]
#define X32_int_enable		peripherals[PERIPHERAL_INT_ENABLE]


//Global log file (64kB)
#define LOG_FILE_SIZE	0xFFFF
unsigned char logfile[LOG_FILE_SIZE];
int logindex;
char logflag;

void log_start() {
	//Initialize
	logindex = 0;
	logflag = 1;

	//Log first event
	log_event(LOG_START);
}
void log_stop() {
	//Log last event
	log_event(LOG_STOP);
	//Clear flag
	logflag = 0;
}

//TODO: Inline possible?
void write_to_log(log_type type, unsigned char* data, int len) {
	//Declare Variables
	int timestamp;
	int reenable;

	if (!logflag)
		return;

	//Critical section
	//Check if global interreupts are already disabled
	if (X32_int_enable & INTERRUPT_GLOBAL) {
		reenable = 0;
	} else {
		reenable = 1;
		DISABLE_INTERRUPT(INTERRUPT_GLOBAL);
	}

	//Write timestamp
	//TODO: Shorter?
	timestamp = X32_ms_clock;
	memcpy(&logfile[logindex], &timestamp, 4);
	logindex += 4;

	//Write type
	logfile[logindex] = type;
	logindex += 1;

	//Write data
	if (len > 0) {
		memcpy(&logfile[logindex], data, len);
		logindex += len;
	}

	//Exit critical section
	if (reenable)
		ENABLE_INTERRUPT(INTERRUPT_GLOBAL);
}

void log_msg(const char* msg) {
	//Declare Variables
	int len;
	unsigned char* data;

	//Find string length
	len = strlen(msg);
	if (len > 255)
		len = 255;

	//Create data part
	data = malloc(len+1);
	data[0] = len;
	memcpy(&data[1], msg, len);
	
	//Write to log
	write_to_log(LOG_MSG, data, len+1);

	//Free memory
	free(data);
}
void log_data(log_type type, unsigned char* data, int len) {
	//Declare Variables
	unsigned char* buffer;

	//Create data part
	buffer = malloc(len+1);
	buffer[0] = len;
	memcpy(&buffer[1], data, len);
	
	//Write to log
	write_to_log(type, buffer, len+1);

	//Free memory
	free(buffer);
}
/*
	These functions were replaced by macros
*/
void log_event(log_type event) {
	//Write to log
	write_to_log(event, 0, 0);
}
void log_int(int value) {
	//Write to log
	write_to_log(LOG_INT, (unsigned char*) &value, 4);
}
void log_byte(unsigned char c) {
	//Write to log
	write_to_log(LOG_BYTE, &c, 1);
}


void log_transmit() {
	//Declare Variables
	unsigned char c;
	comm_type type;
	unsigned char* data;
	int len;
	int index;
	unsigned char buffer[CHUNK_SIZE_7BIT];

	//Log a transmit event
	log_event(LOG_TRANSMIT);

	//Stop logging to prevent logfile corruption
	log_event(LOG_STOP);
	logflag = 0;

	//Send logfile size to sendable format
	len = convert8to7bitint(logindex);

	if (0 != send_data(LOG_SIZE, (unsigned char*) &len, 4)) {
		//TODO: Handle error
		return;
	}

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

		//Convert parameter to integer
		memcpy(&index, data, 4);
		index = convert7to8bitint(index);

		//Free allocated memory from recv_data
		free(data);

		//Convert specific chunk to communicable format
		if (logindex - index > CHUNK_SIZE_8BIT)
			convert8to7bitchunk(&logfile[index], CHUNK_SIZE_8BIT, buffer);
		else
			convert8to7bitchunk(&logfile[index], logindex - index, buffer);

		//Send the chunk
		if (0 != send_data(LOG_CHUNK, buffer, CHUNK_SIZE_7BIT)) {
			//TODO: Handle error
			break;
		}
	}

	//Reactivate log
	logflag = 1;
	log_event(LOG_START);
}
