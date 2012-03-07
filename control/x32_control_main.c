#include <stdio.h>
#include <stdlib.h>

#include "x32_common.h"
#include "comm.h"
#include "x32_modes.h"

void toggleLED (int x);

int kb_lift, kb_yaw, kb_pitch, kb_roll;
int js_lift, js_yaw, js_pitch, js_roll;
int lift, roll, pitch, yaw;
int qr_a0, qr_a1, qr_a2, qr_a3;
comm_type mode, type;
volatile int finished;

int oo1, oo2, oo3, oo4;
int s0, s1, s2, s3, s4, s5, timestamp;
int p_yaw, yaw_error;

unsigned char* data;
int len;

void main(void) {
	//Declare variables
	int result=0;

	// prepare QR rx interrupt handler and associated variables
	 
        SET_INTERRUPT_VECTOR(INTERRUPT_XUFO, &isr_qr_link);
        SET_INTERRUPT_PRIORITY(INTERRUPT_XUFO, 21);
        ENABLE_INTERRUPT(INTERRUPT_XUFO);

	//An interrupt is raised (INTERRUPT_XUFO) once the 7 new values are
	//available to the X32. As an additional feature, there is also a counter register (PERIPHERAL_XUFO_COUNT) that
	//holds the number of times a 7-value frame has been received (every 1/(1,270) ≈ 0.7 ms). In order to facilitate
	//accurate sensor data logging, the counter register PERIPHERAL_XUFO_TIMESTAMP holds the 32 bit timestamp of
	//the last received sensor data frame (in units of 20 ns, based on the X32’s 50 MHz clock).
	//number of sensors 6 + state battery = 7 values
 
        // prepare timer interrupt for 500 Hz engine control (i.e., 5 ms)
        X32_timer_per = 1 * CLOCKS_PER_MS;         
/*        X32_timer_per = 2 * CLOCKS_PER_MS;*/
        SET_INTERRUPT_VECTOR(INTERRUPT_TIMER1, &isr_timer);
        SET_INTERRUPT_PRIORITY(INTERRUPT_TIMER1, 5);
        ENABLE_INTERRUPT(INTERRUPT_TIMER1);

	init_state();
	mode = PANIC; //override init state
	oo1 = 1000;
	
	//Initialise communication
	if (0 != comm_init())
		return;

	ENABLE_INTERRUPT(INTERRUPT_GLOBAL);

	finished=0;	

	X32_leds =0x1;
	X32_display=oo1;
	//Wait until data can be received
	while (!finished)
	{	
		X32_leds |=0x2;
		result = recv_data(&type, &data, &len);
		if ( result == 1) {
			handleInput(type);
			free(data);
		}
	        if (X32_ms_clock % 500 == 0) {
		        toggleLED(128);
			handleMode();
	        	X32_display = oo1;
        	}
	}
	//Uninitialise
	comm_uninit();
}


void init_state(void)
{
	lift = roll = pitch = yaw = 0;
	oo1 = oo2 = oo3 = oo4 = 0;
	p_yaw=0;
	mode = SAFE; //starts in safe mode
}




void handleInput (comm_type type) {
	switch (type) 
	{		
		case RPYL: /* received RPYL info */
			roll = *data++;
			pitch = *data++;
			yaw = *data++;
			lift = *data;
			break;
		case KEYESC: /* ESC: abort / exit */
			break;
		case KEYRETURN: /*increment control mode */
			break;
		case SAFE: // Safe Mode
			if ((roll==0) && (yaw==0) && (pitch==0) && (lift==0))
				mode=SAFE;
			else 
				printf("Error roll, pitch, yaw and lift not zero, can't switch\n");
			break;
		case PANIC: // Panic Mode
			if ((roll==0) && (yaw==0) && (pitch==0) && (lift==0))
				mode=PANIC;
			else 
				printf("Error roll, pitch, yaw and lift not zero, can't switch\n");
			break;
		case MANUAL: // Manual Mode
			if ((roll==0) && (yaw==0) && (pitch==0) && (lift==0))
				mode=MANUAL;
			else 
				printf("Error roll, pitch, yaw and lift not zero, can't switch\n");
			break;
		case CALIBRATION: // Calibration Mode
			if ((roll==0) && (yaw==0) && (pitch==0) && (lift==0))
				mode=CALIBRATION;
			else 
				printf("Error roll, pitch, yaw and lift not zero, can't switch\n");			
			break;
		case YAW: // Yaw control Mode
			if ((roll==0) && (yaw==0) && (pitch==0) && (lift==0))
				mode=YAW;
			else 
				printf("Error roll, pitch, yaw and lift not zero, can't switch\n");
			break;
		case FULL: // Full control Mode
			if ((roll==0) && (yaw==0) && (pitch==0) && (lift==0))
				mode=FULL;
			else 
				printf("Error roll, pitch, yaw and lift not zero, can't switch\n");
			break;		
			
		/* quad rotor control*/	

		case KEYA: /* increase lift */
      			break;
		case KEYZ: /* decrease lift */
      			break;
		case KEYRIGHT: /*right arrow: roll down maybe*/
			break;
		case KEYLEFT: /*left arrow: roll up maybe*/
			break;
		case KEYUP: /*up arrow: pitch down */
			break;
		case KEYDOWN: /*down arrow: pitch up */
			break;
		case KEYW: /* increase yaw */
      			break;
		case KEYQ: /* decrease yaw */
      			break;
		case KEYU: /*yaw control P up*/
			p_yaw++;
			break;
		case KEYJ: /*yaw control P down*/
			p_yaw--;
			break;
		case KEYI: /*roll/pitch control P1 up*/
			break;
		case KEYK: /*roll/pitch control P1 down*/
			break;
		case KEYO: /*roll/pitch control P2 up*/
			break;
		case KEYL: /*roll/pitch control P2 down*/
			break;
		default:
			break;
	}
}


void handleMode (void) {
	switch(mode) {
		case SAFE:
			safe_mode_ctrl();
			break;
		case PANIC:
			panic_mode_ctrl();
			break;
		case MANUAL:
			manual_mode_ctrl();
			break;
		case CALIBRATION:
			calibration_mode();
			break;
		case YAW:
			yaw_mode_ctrl();
			break;
		case FULL:
			full_mode_ctrl();
			break;
		default:
			break;
	}
}

void toggleLED (int x) {
	X32_leds ^= x;	
}



void isr_timer(void)
{
	
	//printf("%d %d %d %d", oo1, oo2, oo3, oo4);
        /* send actuator values to ae0..3 QR peripheral regs
         */

/*	if (oo1<0) oo1=0;*/
/*	if (oo2<0) oo2=0;*/
/*	if (oo3<0) oo3=0;*/
/*	if (oo4<0) oo4=0;*/
        
	X32_QR_a0 = oo1;
	//X32_display = oo1;
        X32_QR_a1 = oo2;
        X32_QR_a2 = oo3;
        X32_QR_a3 = oo4;
}


 /*------------------------------------------------------------------
 * isr_qr_link -- QR link rx interrupt handler
 *------------------------------------------------------------------
 */
void isr_qr_link(void)
{
	/* get sensor and timestamp values
	 */
	s0 = X32_QR_s0; s1 = X32_QR_s1; s2 = X32_QR_s2; 
	s3 = X32_QR_s3; s4 = X32_QR_s4; s5 = X32_QR_s5;
}
