#include "x32_common.h"
#include "x32_log.h"
#include "serial.h"
#include <stdio.h>
#include <string.h>

#define X32_ms_clock		peripherals[PERIPHERAL_MS_CLOCK]
#define X32_int_enable		peripherals[PERIPHERAL_INT_ENABLE]
#define X32_leds		peripherals[PERIPHERAL_LEDS]
#define X32_display		peripherals[PERIPHERAL_DISPLAY]

//Global log file (64kB)
#define LOG_FILE_SIZE	0xFFFF
char logfile[LOG_FILE_SIZE];
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
	log_write("Start logging\n");
}

/*
	Stop logging
*/
void log_stop() {
	//Log last event
	log_write("stop Logging\n");
	//Clear flag
	logflag = 0;
}

/*
	Apply changes to the log file
	Input:
		type	type of log entry (see log.h)
		data	data buffer
		len	length of data in buffer
*/
void log_write(char* data) {
	//Declare Variables
	char reenable;

	//Critical section
	//Check if global interrupts are already disabled
	if (X32_int_enable & INTERRUPT_GLOBAL) {
		reenable = 0;
	} else {
		reenable = 1;
		DISABLE_INTERRUPT(INTERRUPT_GLOBAL);
	}

	//Write time stamp and data	
	if (logflag)
		logindex += sprintf(logfile+logindex, "%d %s", X32_ms_clock, data);

	//Exit critical section
	if (reenable)
		ENABLE_INTERRUPT(INTERRUPT_GLOBAL);
}

void log_retrieve(void) {
	//Declare Variables
	int i;
	int result;

	//Log a transmit event
	log_write("Retrieving log\n");


	//Stop logging to prevent log file corruption
	log_stop();
		

	X32_display = logindex;
	
	//Send data
	i=0;
	result=0;
	logfile[logindex] = 'X'; //append X to logfile has a termination character
/*	Returns 1 if byte was written successfully, 0 if busy, -1 on error.*/
	while (i <= logindex) {
		result = serial_write(logfile[i]);
		if (result == 0 || result == 1)
			i++;
	}
	
/*	for(i=0; i < logindex;i++)*/
/*		printf("%c",logfile[i]);*/

	//Reactivate log
	log_start();
}
