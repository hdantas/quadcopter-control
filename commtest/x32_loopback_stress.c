#include <stdio.h>
#include <stdlib.h>

#include "x32_common.h"
#include "comm.h"

#define X32_timer_per           peripherals[PERIPHERAL_TIMER1_PERIOD]
#define X32_leds		peripherals[PERIPHERAL_LEDS]
#define X32_ms_clock		peripherals[PERIPHERAL_MS_CLOCK]
#define X32_us_clock		peripherals[PERIPHERAL_US_CLOCK]
#define X32_QR_a0 		peripherals[PERIPHERAL_XUFO_A0]
#define X32_QR_a1 		peripherals[PERIPHERAL_XUFO_A1]
#define X32_QR_a2 		peripherals[PERIPHERAL_XUFO_A2]
#define X32_QR_a3 		peripherals[PERIPHERAL_XUFO_A3]
#define X32_QR_s0 		peripherals[PERIPHERAL_XUFO_S0]
#define X32_QR_s1 		peripherals[PERIPHERAL_XUFO_S1]
#define X32_QR_s2 		peripherals[PERIPHERAL_XUFO_S2]
#define X32_QR_s3 		peripherals[PERIPHERAL_XUFO_S3]
#define X32_QR_s4 		peripherals[PERIPHERAL_XUFO_S4]
#define X32_QR_s5 		peripherals[PERIPHERAL_XUFO_S5]
#define X32_QR_timestamp 	peripherals[PERIPHERAL_XUFO_TIMESTAMP]
#define X32_display		peripherals[PERIPHERAL_DISPLAY]

int finished;

void toggle_led(int i) 
{
	X32_leds = (X32_leds ^ (1 << i));
}

void main(void) {
	//Declare variables
	comm_type type;
	unsigned char* data;
	int len;
	int I;
	int result;

	//Initialise communication
	if (0 != comm_init())
		return;

	ENABLE_INTERRUPT(INTERRUPT_GLOBAL);

	//Initialise
	finished=0;	

	X32_leds =0x0;

	//Wait until data can be received
	while (!finished)
	{	
		if(1 == recv_data(&type, &data, &len)) 
		{		
			if (0 != send_data(type , data, len))
			{
				toggle_led(7);
				break;
			}
			//Free buffer memory again (IMPORTANT!)
			free(data);
		}

		
	}

	//Uninitialise
	comm_uninit();
}

