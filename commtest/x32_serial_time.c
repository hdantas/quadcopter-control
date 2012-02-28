//I edit the line with TODO to allow the measurament of time
//I use it only in pc_stress end x32_stress. in other case you must use X32_serial

#include "serial.h"

#include "x32_nexys.h"
#include "x32_common.h"
#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>

#define X32_serial_data		peripherals[PERIPHERAL_PRIMARY_DATA]
#define X32_serial_status	peripherals[PERIPHERAL_PRIMARY_STATUS]
#define X32_leds		peripherals[PERIPHERAL_LEDS]
#define X32_display		peripherals[PERIPHERAL_DISPLAY]

extern int start_time;	//TODO

//Local ISRs
void isr_serial_rx(void);
void isr_serial_tx(void);

//Circular buffers
#define SERIAL_BUFFER_SIZE	16
unsigned char serial_buffer_recv[SERIAL_BUFFER_SIZE];
int serial_buffer_recv_base;
int serial_buffer_recv_top;
unsigned char serial_buffer_send[SERIAL_BUFFER_SIZE];
int serial_buffer_send_base;
int serial_buffer_send_top;

void toggle_led(int i) 
{
	//X32_leds = (X32_leds ^ (1 << i));
}

int serial_init() {
	//Declare variables
	unsigned char c;
	
	//Setup interrupts (recv)
	SET_INTERRUPT_VECTOR(INTERRUPT_PRIMARY_RX, &isr_serial_rx);
	SET_INTERRUPT_PRIORITY(INTERRUPT_PRIMARY_RX, 20);
	
	//There may yet be data in the internal buffer, empty to prevent unscheduled interrupts
	while (X32_serial_status & 0x02)
		//Emtpy through reading
		c = X32_serial_data;

	//Setup interrupts (send)
	SET_INTERRUPT_VECTOR(INTERRUPT_PRIMARY_TX, &isr_serial_tx);
	SET_INTERRUPT_PRIORITY(INTERRUPT_PRIMARY_TX, 15);

	//Clear buffers
	serial_buffer_recv_base = 0;
	serial_buffer_recv_top = 0;
	serial_buffer_send_base = 0;
	serial_buffer_send_top = 0;
	
	//Enable  interrupts
	ENABLE_INTERRUPT(INTERRUPT_PRIMARY_RX);
	ENABLE_INTERRUPT(INTERRUPT_PRIMARY_TX);

	//Communication is setup correctly
	//X32_leds |= COMM_LED;

	return 0;
}

void serial_uninit() {
	//Disable interrupts
	DISABLE_INTERRUPT(INTERRUPT_PRIMARY_RX);
	DISABLE_INTERRUPT(INTERRUPT_PRIMARY_TX);

	//Communication is off
	//X32_leds &= ~COMM_LED;
}

/*	Returns 1 if data is available, 0 otherwise 
	Data itself is placed in *buffer;
*/
int serial_read(unsigned char* buffer) {
	//Check if buffer holds data

	if (serial_buffer_recv_base != serial_buffer_recv_top) {
		//Return a character and update buffer
		*buffer = serial_buffer_recv[serial_buffer_recv_base];
		serial_buffer_recv_base++;
		if (serial_buffer_recv_base >= SERIAL_BUFFER_SIZE)
			serial_buffer_recv_base = 0;
		return 1;
	} else {
		//No data to be read
		return 0;
	}
}

/*	Returns 1 if data was written directly, 0 if placed in buffer, -1 otherwise
*/
int serial_write(unsigned char c) {
	//Check if data can be written immediately
	if ((serial_buffer_send_top == serial_buffer_send_base) &&
		(X32_serial_status & 0x01)) {

		//Write to buffer, and done
		X32_serial_data = c;
		return 1;
	} else {
		//Place data in buffer, update, *critical section due to top/base comparison*
		DISABLE_INTERRUPT(INTERRUPT_GLOBAL);
		serial_buffer_send[serial_buffer_send_top] = c;
		serial_buffer_send_top++;
		if (serial_buffer_send_top >= SERIAL_BUFFER_SIZE)
			serial_buffer_send_top = 0;
/*		if (serial_buffer_send_top == serial_buffer_send_base) {*/
/*			log_msg("Serial output buffer full; discarding");*/
/*			log_int(serial_buffer_send_top);*/
/*			log_data(LOG_COMM_DISCARD_BUFFER, serial_buffer_send, SERIAL_BUFFER_SIZE);*/
/*			return -1;*/
/*		}*/
		ENABLE_INTERRUPT(INTERRUPT_GLOBAL);
		return 0;
	}
}

//ISRs
void isr_serial_rx(void) {
	start_time=peripherals[PERIPHERAL_US_CLOCK]; //TODO
	//Keep reading while flag is raised	
	while (X32_serial_status & 0x02) {
		//X32_leds=serial_buffer_recv_top;
		//Read into buffer, update
		serial_buffer_recv[serial_buffer_recv_top]= X32_serial_data;
		//X32_display=serial_buffer_recv[serial_buffer_recv_top];
		//Update and circle index
		serial_buffer_recv_top++;
		if (serial_buffer_recv_top >= SERIAL_BUFFER_SIZE)
			serial_buffer_recv_top = 0;
/*		if (serial_buffer_recv_top == serial_buffer_recv_base) {*/
/*			log_msg("Serial input buffer full; discarding");*/
/*			log_int(serial_buffer_recv_top);*/
/*			log_data(LOG_COMM_DISCARD_BUFFER, serial_buffer_recv, SERIAL_BUFFER_SIZE);*/
/*		}*/
		//X32_leds=serial_buffer_recv[serial_buffer_recv_top];
	}
}
void isr_serial_tx(void) {
	//Check flag and buffer contents
	if ((X32_serial_status & 0x01) &&
		(serial_buffer_send_top != serial_buffer_send_base)) {
		//Write to output buffer
		X32_serial_data = serial_buffer_send[serial_buffer_send_base];
		//X32_display = serial_buffer_send[serial_buffer_send_base];
		//Update and circle index
		serial_buffer_send_base++;
		if (serial_buffer_send_base >= SERIAL_BUFFER_SIZE)
			serial_buffer_send_base = 0;
	}
}
