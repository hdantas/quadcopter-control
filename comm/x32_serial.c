#include "serial.h"

//#include "x32_nexys.h"
#include "x32_common.h"
#include "log.h"
#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>

#define X32_serial_data		peripherals[PERIPHERAL_PRIMARY_DATA]
#define X32_serial_status	peripherals[PERIPHERAL_PRIMARY_STATUS]
#define X32_leds		peripherals[PERIPHERAL_LEDS]
//#define X32_display		peripherals[PERIPHERAL_DISPLAY]
		

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

/*	Initialise the serial connection on the QR-side
 *	Author: Maurijn Neumann
 */
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
	X32_leds |= COMM_LED;

	return 0;
}

/*	Uninitialise the serial connection on the QR-side
 *	Author: Maurijn Neumann
 */
void serial_uninit() {
	//Disable interrupts
	DISABLE_INTERRUPT(INTERRUPT_PRIMARY_RX);
	DISABLE_INTERRUPT(INTERRUPT_PRIMARY_TX);

	//Communication is off
	X32_leds &= ~COMM_LED;
}

/*	Attempt to retrieve a byte of data that has come in from the serial link (QR-side)
 *	Returns 1 if data is available, 0 otherwise 
 *	Data itself is placed in *buffer;
 *	Author: Maurijn Neumann
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

/*	Write a byte of data to the serial link or set it up to be sent later (QR-side)
 *	Returns 1 if data was written directly, 0 if placed in buffer, -1 otherwise
 *	Author: Maurijn Neumann
 */
int serial_write(unsigned char c) {
	//Check if data can be written immediately
	if ((serial_buffer_send_top == serial_buffer_send_base) &&
		(X32_serial_status & 0x01)) {

		//Write to buffer, and done
//		X32_display = c;
		X32_serial_data = c;
		return 1;
	} else {
		//Place data in buffer, update, *critical section due to top/base comparison*
		DISABLE_INTERRUPT(INTERRUPT_GLOBAL);
		serial_buffer_send[serial_buffer_send_top] = c;
		serial_buffer_send_top++;
		if (serial_buffer_send_top >= SERIAL_BUFFER_SIZE)
			serial_buffer_send_top = 0;
		if (serial_buffer_send_top == serial_buffer_send_base) {
			log_msg("Serial output buffer full; discarding");
			log_int(serial_buffer_send_top);
			log_data(LOG_COMM_DISCARD_BUFFER, serial_buffer_send, SERIAL_BUFFER_SIZE);
			return -1;
		}
		ENABLE_INTERRUPT(INTERRUPT_GLOBAL);
		return 0;
	}
}

//ISRs
/*	ISR for incoming serial data on the QR-side
 *	Author: Maurijn Neumann
 */
void isr_serial_rx(void) {
	//Keep reading while flag is raised
	while (X32_serial_status & 0x02) {
		//Read into buffer, update
		serial_buffer_recv[serial_buffer_recv_top] = X32_serial_data;
		//Update and circle index
		serial_buffer_recv_top++;
		if (serial_buffer_recv_top >= SERIAL_BUFFER_SIZE)
			serial_buffer_recv_top = 0;
		if (serial_buffer_recv_top == serial_buffer_recv_base) {
			log_msg("Serial input buffer full; discarding");
			log_int(serial_buffer_recv_top);
			log_data(LOG_COMM_DISCARD_BUFFER, serial_buffer_recv, SERIAL_BUFFER_SIZE);
		}
	}
}
/*	ISR for outgoing data on the serial link on the QR-side
 *	Author: Maurijn Neumann
 */
void isr_serial_tx(void) {
	//Check flag and buffer contents
	if ((X32_serial_status & 0x01) &&
		(serial_buffer_send_top != serial_buffer_send_base)) {
		//Write to output buffer
		X32_serial_data = serial_buffer_send[serial_buffer_send_base];
		//Update and circle index
		serial_buffer_send_base++;
		if (serial_buffer_send_base >= SERIAL_BUFFER_SIZE)
			serial_buffer_send_base = 0;
	}
}
