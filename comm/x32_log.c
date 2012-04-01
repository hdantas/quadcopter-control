#include "log.h"
#include "x32_log.h"
#include "x32_common.h"
//#include "convert.h"
#include "comm.h"
#include "serial.h"

#include <stdlib.h>
#include <string.h>

void displayData(unsigned char* data, int len);


#define X32_ms_clock		peripherals[PERIPHERAL_MS_CLOCK]
#define X32_us_clock		peripherals[PERIPHERAL_US_CLOCK]
#define X32_int_enable		peripherals[PERIPHERAL_INT_ENABLE]
#define X32_leds			peripherals[PERIPHERAL_LEDS]
#define X32_display			peripherals[PERIPHERAL_DISPLAY]


//Global log file (~400kB)
#define LOG_FILE_SIZE	0x5FFFF
#define LOG_FILE_LIMIT	LOG_FILE_SIZE - 128
unsigned char logfile[LOG_FILE_SIZE];
int logindex = 0;
char logflag = 0;

/*	Start logging
 *	Author: Maurijn Neumann
 */
void log_start() {
	//Clear logfile
	for (logindex = 0; logindex < LOG_FILE_SIZE; logindex++)
		logfile[logindex] = 0x88;
		
	//Initialize
	logindex = 0;
	logflag = 1;

	//Log first event
	log_event(LOG_START);
}
/*	Stop logging
 *	Author: Maurijn Neumann
 */
void log_stop() {
	//Log last event
	log_event(LOG_STOP);
	//Clear flag
	logflag = 0;
}

/*	Apply changes to the logfile on the QR-side
 *	Input:
 *		type	type of log entry (see log.h)
 *		data	data buffer
 *		len		length of data in buffer
 *	Author: Maurijn Neumann
 */
void write_to_log(log_type type, unsigned char* data, int len) {
	//Declare Variables
	unsigned int timestamp;
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
	timestamp = X32_us_clock;

	logfile[logindex] = (timestamp >> 24) & 0x000000FF;
	logindex++;
	logfile[logindex] = (timestamp >> 16) & 0x000000FF;
	logindex++;
	logfile[logindex] = (timestamp >> 8) & 0x000000FF;
	logindex++;
	logfile[logindex] = timestamp & 0x000000FF;
	logindex++;
	
	//Write type
	logfile[logindex] = type;
	logindex += 1;

	//Write data
	if (len > 0) {
		memcpy(&logfile[logindex], data, len);
		logindex += len;
	}
	
	//Stop logging once file is full
	if (logindex >= LOG_FILE_LIMIT)
		logflag = 0;
	
	//Exit critical section
	if (reenable)
		ENABLE_INTERRUPT(INTERRUPT_GLOBAL);
}
/*
void write_to_log(log_type type, unsigned char* data, int len) {
	//Declare Variables
	unsigned int timestamp;
	int reenable;
	int I;

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
//	timestamp = 0xFFFFFFFF; //X32_ms_clock;
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
}*/

/*	Shorthand to log a string on the QR-side
 *	Author: Maurijn Neumann
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
/*	Shorthand to log binary data on the QR-side
 *	Author: Maurijn Neumann
 */
void log_data(log_type type, unsigned char* data, int len) {
	//Declare Variables
	unsigned char* buffer;
	unsigned int timestamp;
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
	timestamp = X32_us_clock;

	logfile[logindex] = (timestamp >> 24) & 0x000000FF;
	logindex++;
	logfile[logindex] = (timestamp >> 16) & 0x000000FF;
	logindex++;
	logfile[logindex] = (timestamp >> 8) & 0x000000FF;
	logindex++;
	logfile[logindex] = timestamp & 0x000000FF;
	logindex++;
	
	//Write type
	logfile[logindex] = type;
	logindex += 1;
	
	//Write length
	if (type >= LOG_TYPE_DATA_VAR) {
		logfile[logindex] = len;
		logindex += 1;
	}

	//Write data
	if (len > 0) {
		memcpy(&logfile[logindex], data, len);
		logindex += len;
	}
	
	//Stop logging once file is full
	if (logindex >= LOG_FILE_LIMIT)
		logflag = 0;
	
	//Exit critical section
	if (reenable)
		ENABLE_INTERRUPT(INTERRUPT_GLOBAL);
}

/*
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
}*/
/*	Shorthand to log an event (w/o data) on the QR-side
 *	Author: Maurijn Neumann
 */
void log_event(log_type event) {
	//Write to log
	write_to_log(event, 0, 0);
}
/*	Shorthand to log an integer value on the QR-side
 *	Author: Maurijn Neumann
 */
void log_int(int value) {
	//Write to log
	write_to_log(LOG_INT, (unsigned char*) &value, 4);
}
/*	Shorthand to log a single byte on the QR-side
 *	Author: Maurijn Neumann
 */
void log_byte(unsigned char c) {
	//Write to log
	write_to_log(LOG_BYTE, &c, 1);
}

/*	Transmits the logfile over the serial link. Should be called upon receipt of REQ_LOG packet.
 *	Blocking and ugly.
 *	Author: Maurijn Neumann
 */
void log_transmit() {
	//Declare Variables
	int time;
	int phase;
	int I;
	unsigned char c;

	//Log a transmit event
	log_event(LOG_TRANSMIT);
	
	//Send acknowledgement
	send_data(REQ_LOG, 0, 0);

	//Stop logging to prevent logfile corruption
	log_event(LOG_STOP);
	logflag = 0;
	
	//Delay to allow time for PC-side processing (100ms)
	time = X32_ms_clock;
	while (X32_ms_clock < time+100);

	//Initialize
	phase = 1;
	I = 0;
	
	while (phase < 3) {
		if (phase == 1) {
			//Transmit size
			c = (logindex >> (24 - I * 8)) & 0x000000FF;
			I++;
			if (I == 4) {
				I = 0;
				phase++;
			}
		} else if (phase == 2) {
			//Transmit a log byte
			c = logfile[I];
			I++;
			if (I == logindex) {
				phase++;
			}
		}
		
		//Send
		serial_write(c);
	}

	//Reactivate log
	logflag = 1;
	log_event(LOG_START);
}


/*
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
			free(data);
			break;
		}

		//Retrieve index from data
		index = make_int(data);

		//Free allocated memory from recv_data
		free(data);

		//Convert specific chunk to communicable format
		memset(buffer, 0, 2*CHUNK_SIZE_7BIT);
		if (logindex - index > 2*CHUNK_SIZE_8BIT)
			convert8to7bitchunk(&logfile[index], 2*CHUNK_SIZE_8BIT, buffer);
		else
			convert8to7bitchunk(&logfile[index], logindex - index, buffer);
			
//		displayData(buffer, 2*CHUNK_SIZE_7BIT);

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
*/
/*	Debug function to slowly show data on the embedded display, one byte every second
 *	Author: Maurijn Neumann
 */
void displayData(unsigned char* data, int len) {
	//Declare Variables
	int I;
	unsigned int time;
	
	X32_display = 0;
	X32_leds |= 0x20;
	
	for (I = 0; I < len; I++) {
		X32_display = (I << 8) | data[I];
		
		//Wait 1 second
		time = X32_ms_clock + 1000;
		while (X32_ms_clock < time);
	}
	
	X32_display = 0;
	X32_leds &= ~0x20;
}
