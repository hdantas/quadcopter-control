#include <stdio.h>
#include <stdlib.h>

#include "x32_common.h"
#include "comm.h"
#include "log.h"
#include "x32_log.h"

void main(void) {
	//Declare variables
	unsigned char axes[4];
	comm_type type;
	unsigned char* data;
	int len;
	int I;

	log_start();

	//Initialise communication
	if (0 != comm_init())
		return;

	//Enable interrupts
	ENABLE_INTERRUPT(INTERRUPT_GLOBAL);

	//Everything is active
	peripherals[PERIPHERAL_LEDS] |= STATUS_LED;

	log_msg("QR is ready to receive data...");

	//Loop infinitely
	while (1) {
		//Receive data (make non-blocking blocking)
		while (0 == recv_data(&type, &data, &len));

		if (type == KEYESC) {
			//Done
			free(data);
			break;
		} else if (type == REQLOG) {
			//Send logfile
			peripherals[PERIPHERAL_LEDS] |= 0x80;
			log_transmit();
			peripherals[PERIPHERAL_LEDS] &= 0x7F;
		} else {
			//Send it back
//			send_data(type, data, len);
			log_msg("The quick brown fox jumps over the lazy dog.");
		}
		//Free buffer memory again (IMPORTANT!)
		free(data);
	}

	//Uninitialise
	comm_uninit();

	log_stop();
}
