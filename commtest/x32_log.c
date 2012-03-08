#include "log.h"
#include "x32_log.h"
#include "x32_common.h"
#include "convert.h"
#include "comm.h"

#include <stdlib.h>
#include <string.h>

#define X32_ms_clock		peripherals[PERIPHERAL_MS_CLOCK]
#define X32_int_enable		peripherals[PERIPHERAL_INT_ENABLE]
#define X32_leds			peripherals[PERIPHERAL_LEDS]


//Global log file (64kB)
#define LOG_FILE_SIZE	0xFFFF
unsigned char logfile[LOG_FILE_SIZE];
int logindex;
char logflag;

/*
	Start logging
*/
void log_start() {
	//Initialize
	logindex = 0;
	logflag = 1;

	//Log first event
	log_event(LOG_START);
}
/*
	Stop logging
*/
void log_stop() {
	//Log last event
	log_event(LOG_STOP);
	//Clear flag
	logflag = 0;
}

/*
	Apply changes to the logfile
	Input:
		type	type of log entry (see log.h)
		data	data buffer
		len		length of data in buffer
*/
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

/*
	Shorthand to log a string
*/
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
/*
	Shorthand to log binary data
*/
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
	Shorthand to log an event (w/o data)
*/
void log_event(log_type event) {
	//Write to log
	write_to_log(event, 0, 0);
}
/*
	Shorthand to log an integer value directly
*/
void log_int(int value) {
	//Write to log
	write_to_log(LOG_INT, (unsigned char*) &value, 4);
}
/*
	Shorthand to log a single byte directly
*/
void log_byte(unsigned char c) {
	//Write to log
	write_to_log(LOG_BYTE, &c, 1);
}

/*
	Transmits the logfile over the serial link. Should be called upon receipt of REQ_LOG packet.
	Blocking and ugly.
*/
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
	
	X32_leds |= 0x40;

	//Send logfile size to sendable format
	make_int_sendable(logindex, &intbuffer, &intbufferlen);

	if (0 != send_data(LOG_SIZE, intbuffer, intbufferlen)) {
		//TODO: Handle error
		
		free(intbuffer);
		return;
	} else {
		free(intbuffer);
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

		//Retrieve index from data
		index = make_int(data);

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
