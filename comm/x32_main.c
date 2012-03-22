#include <stdio.h>
#include <stdlib.h>

#include "x32_common.h"
#include "x32_log.h"
#include "serial.h"


int main(void) {
	//Declare variables
	unsigned char* data;


	log_start();

	//Initialise communication
	if (0 != serial_init())
		return -1;

	//Enable interrupts
	ENABLE_INTERRUPT(INTERRUPT_GLOBAL);

	//Everything is active
	peripherals[PERIPHERAL_LEDS] = 0;


	//Loop infinitely
	while (1) {
		//Receive data (make non-blocking blocking)
		while (1 != serial_read(data));

		if (*data == 'E') {
			//Done
			printf("Received ESC.\n");
			log_write("Received ESC.\n");
			break;
		} else if (*data == 'L') {
			printf("Received REQ_LOG.\n");
			log_write("Received REQ_LOG.\n");
			log_write("sensor 1 0.123\n");
			log_write("sensor 2 4.567\n");
			log_write("sensor 3 8.901\n");
			log_write("sensor 4 2.345\n");
			//Send logfile
			//peripherals[PERIPHERAL_LEDS] |= 0x80;
			log_retrieve();
			//peripherals[PERIPHERAL_LEDS] &= 0x7F;
		} else {
			//Send it back
//			send_data(type, data, len);
//			printf("The quick brown fox jumps over the lazy dog.\n");
		}
		//Free buffer memory again (IMPORTANT!)
	}

	//Uninitialise
	serial_uninit();
	log_stop();
	peripherals[PERIPHERAL_LEDS] &= 0x7F;
	return 0;
}
