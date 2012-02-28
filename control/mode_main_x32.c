#include <stdio.h>
#include <stdlib.h>

#include "x32_common.h"
#include "comm.h"
#include "modes_x32.h"

void toggleLED (int x);

int kb_lift, kb_yaw, kb_pitch, kb_roll;
int js_lift, js_yaw, js_pitch, js_roll;
int lift, roll, pitch, yaw;
int qr_a0, qr_a1, qr_a2, qr_a3;
comm_type mode, type;
volatile int finished;


unsigned char* data;
int len;

void main(void) {
	//Declare variables
	int result=0;



	// prepare QR rx interrupt handler and associated variables
	 
//        SET_INTERRUPT_VECTOR(INTERRUPT_XUFO, &isr_qr_link);
//        SET_INTERRUPT_PRIORITY(INTERRUPT_XUFO, 21);
//        ENABLE_INTERRUPT(INTERRUPT_XUFO);

	//An interrupt is raised (INTERRUPT_XUFO) once the 7 new values are
	//available to the X32. As an additional feature, there is also a counter register (PERIPHERAL_XUFO_COUNT) that
	//holds the number of times a 7-value frame has been received (every 1/(1,270) ≈ 0.7 ms). In order to facilitate
	//accurate sensor data logging, the counter register PERIPHERAL_XUFO_TIMESTAMP holds the 32 bit timestamp of
	//the last received sensor data frame (in units of 20 ns, based on the X32’s 50 MHz clock).
	//number of sensors 6 + state battery = 7 values
 
        // prepare timer interrupt for 500 Hz engine control (i.e., 5 ms)
        X32_timer_per = 2 * CLOCKS_PER_MS;         
/*        X32_timer_per = 2 * CLOCKS_PER_MS;*/
        SET_INTERRUPT_VECTOR(INTERRUPT_TIMER1, &isr_timer);
        SET_INTERRUPT_PRIORITY(INTERRUPT_TIMER1, 5);
        ENABLE_INTERRUPT(INTERRUPT_TIMER1);

	init_state();

	//Initialise communication
	if (0 != comm_init())
		return;

	ENABLE_INTERRUPT(INTERRUPT_GLOBAL);


	finished=0;	

	X32_leds =0x1;
	X32_display=0x1234;
	//Wait until data can be received
	while (!finished)
	{	
		X32_leds |=0x2;
		result = recv_data(&type, &data, &len);
		if ( result == 1) {
			type = FULL;
			X32_display = len;
			toggleLED(0x04);
			if (type == FULL) {
				toggleLED(0x08);
				handleMODE();
			}
			else if (len==4 && type==RPYL)
				handleRPYL();
			//Free buffer memory again (IMPORTANT!)
			free(data);
		}
	}
	//Uninitialise
	comm_uninit();
}


void init_state(void)
{
	lift = roll = pitch = yaw = 0;
	qr_a0 = qr_a1 = qr_a2 = qr_a3 = 0;
	safe_mode_ctrl(); //starts in safe mode
}

void isr_timer(void)
{
	int clk = X32_MS_CLOCK;
	
	//printf("%d %d %d %d", qr_a0, qr_a1, qr_a2, qr_a3);
        // send actuator values to ae0..3 QR peripheral regs
                
	X32_QR_a0 = qr_a0;
	//X32_display = qr_a0;
        X32_QR_a1 = qr_a1;
        X32_QR_a2 = qr_a2;
        X32_QR_a3 = qr_a3;

	
	if ((clk % 8000) == 0)
		toggleLED(16+32+64+128);
	else if ((clk % 2000) == 0)
		toggleLED(16+32+64);
	else if ((clk % 500) == 0)
		toggleLED(16+32);
	else if ((clk % 125) == 0)
		toggleLED(16);
}

void handleMODE (void) {
	mode = type;
	switch (mode) 
	{		
		case SAFE: // Safe Mode
			X32_display=0x1;
			safe_mode_ctrl();
			break;
		case PANIC: // Panic Mode
			X32_display=0x2;
			panic_mode_ctrl();
			break;
		case MANUAL: // Manual Mode
			X32_display=0x3;
			manual_mode_ctrl();
			break;
		case CALIBRATION: // Calibration Mode
			X32_display=0x4;
			calibration_mode();
			break;
		case YAW: // Yaw control Mode
			X32_display=0x5;
			yaw_mode_ctrl();
			break;
		case FULL: // Full control Mode
			X32_display=0x6;
			full_mode_ctrl();
			break;		
		default:
			X32_display=0x3333;
			break;
	}
}

void handleRPYL (void) {
	roll = *data++;
	pitch = *data++;
	yaw = *data++;
	lift = *data;
}

void toggleLED (int x) {
	X32_leds ^= x;	
}

