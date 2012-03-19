#include <stdio.h>
#include <stdlib.h>

#include "x32_common.h"
#include "comm.h"
#include "x32_modes.h"
#include "kalman.h"

/*int kb_lift, kb_yaw, kb_pitch, kb_roll;*/
/*int js_lift, js_yaw, js_pitch, js_roll;*/
int lift, roll, pitch, yaw;
int oldlift, oldroll, oldpitch, oldyaw;
comm_type mode, type;
volatile int finished;

int oo1, oo2, oo3, oo4;
int s0, s1, s2, s3, s4, s5, timestamp;
int s0_bias, s1_bias, s2_bias, s3_bias, s4_bias, s5_bias;
int p_yaw, yaw_controller;
int p1_full, p2_full;

unsigned char* data;
int len;

void main(void) {

	init_state();
	 
	SET_INTERRUPT_VECTOR(INTERRUPT_XUFO, &isr_qr_link);
	SET_INTERRUPT_PRIORITY(INTERRUPT_XUFO, 21);
	ENABLE_INTERRUPT(INTERRUPT_XUFO);

	// prepare timer interrupt for 500 Hz engine control (i.e., 5 ms)
	X32_timer_per = 1 * CLOCKS_PER_MS;         
	SET_INTERRUPT_VECTOR(INTERRUPT_TIMER1, &isr_timer);
	SET_INTERRUPT_PRIORITY(INTERRUPT_TIMER1, 5);
	ENABLE_INTERRUPT(INTERRUPT_TIMER1);


	//Initialise communication
	if (0 != comm_init())
		return;

	ENABLE_INTERRUPT(INTERRUPT_GLOBAL);

	finished=0;	

	//Wait until data can be received
	while (!finished)
	{	
		if ( recv_data(&type, &data, &len) == 1) {
			handleInput();
			free(data);
		}
	if (X32_ms_clock % 100 == 0){
		X32_display = oo1;
	}
	handleMode();		
	compute_RPMs(); //compute the new oo* values based on the defined RPYL

	}
	//Uninitialise
	comm_uninit();
}


void init_state(void)
{
	lift = roll = pitch = yaw = 0;
	oo1 = oo2 = oo3 = oo4 = 0;
	p_yaw = INITIAL_P_YAW;
	yaw_controller = 0;
	p1_full = INITIAL_P1_FULL;
	p2_full = INITIAL_P2_FULL;		
	s0 = s1 = s2 = s3 = s4 = s5 = 0;
	s0_bias = s1_bias = s2_bias = s3_bias = s4_bias = s5_bias = 0;
	mode = INITIAL_MODE; //starts in INITIAL_STATE mode
	init_kalman();
}


void handleInput (void) {
	if (type == KEY1) {
		mode = PANIC;
		return;
	}
	
	if (type == RPYL) {
		roll = data[0] - 64;
		pitch = data[1] - 64;
		yaw = data[2] - 64;
		lift = data[3];
		return;
	}
	
	if (type >= KEYY && type <= KEYO){ //Control Parameter changes
		switch (type) {
			case KEYY: ////increase P2PHI
				p2phi += UP_P2PHI;
				if (p2phi > MAX_P2PHI) p2phi = MAX_P2PHI;
				printf("Increase P2PHI\n");
				break;
			case KEYH: //decrease P2PHI
				p2phi -= DOWN_P2PHI;
				if (p2phi < MIN_P2PHI) p2phi = MIN_P2PHI;
				printf("Decrease P2PHI\n");
				break;			
			case KEYU: /*yaw control P up*/
				p_yaw += UP_P_YAW;
				if (p_yaw > MAX_P_YAW) p_yaw = MAX_P_YAW;
				printf("Increase yaw control P\n");
				break;
			case KEYJ: /*yaw control P down*/
				p_yaw -= DOWN_P_YAW;
				if (p_yaw < MIN_P_YAW) p_yaw = MIN_P_YAW;
				printf("Decrease yaw control P\n");
				break;
			case KEYI: //roll/pitch control P1 up
				p1_full += UP_P1_FULL;
				if (p1_full > MAX_P1_FULL) p1_full = MAX_P1_FULL;
				printf("Increase roll pitch control P1\n");
				break;
			case KEYK: //roll/pitch control P1 down
				p1_full -= DOWN_P1_FULL;
				if (p1_full < MIN_P1_FULL) p1_full = MIN_P1_FULL;
				printf("Decrease roll pitch control P1\n");
				break;
			case KEYO: //roll/pitch control P2 up
				p2_full += UP_P2_FULL;
				if (p2_full > MAX_P2_FULL) p2_full = MAX_P2_FULL;				
				printf("Increase roll pitch control P2\n");
				break;
			case KEYL: //roll/pitch control P2 down
				p2_full -= DOWN_P2_FULL;
				if (p2_full < MIN_P2_FULL) p2_full = MIN_P2_FULL;
				printf("Decrease roll pitch control P1\n");
				break;
			default:
				printf("Doesn't know what controller parameter to change\n");
				break;				
		}
		printf("Controller changes:\np_yaw = %d\tp1_full = %d\tp2_full = %d\tp2phi = %d\n", p_yaw,p1_full,p2_full,p2phi);
		return;
	}
		
	if ((oo1==0) && (oo2==0) && (oo3==0) && (oo4==0)) //Change control
	{		
		calibration_mode();
	  	switch (type) {			
				case KEYESC: /* ESC: abort / exit */	
					printf("Exiting...\n");			
					finished=1;				
					break;		
				case KEYRETURN: /*increment control mode */
					if (mode != FULL)
						mode++;
					else
						mode=SAFE;				
					printf("Changing to next mode\n");
					break;		
				case KEY0: /*Safe Mode*/
					mode=SAFE;
					printf("Changing to safe mode\n");
					break;
				case KEY2: /*Manual Mode*/
					mode=MANUAL;	
					printf("Changing to manual mode\n");
					break;										
				case KEY3: /*Calibration Mode*/
					mode=CALIBRATION;
					printf("Changing to calibration mode\n");
					break;					
				case KEY4: /*Yaw control Mode*/
					mode=YAW;
					yaw_controller = yaw; //save the yaw to use in the controller;
					printf("Changing to yaw mode\n");
					break;
				case KEY5: /*Full control mode*/
					mode=FULL;
					printf("Changing to full mode\n");
					break;
				default:
					printf("Ready to change mode but wrong key.\n");
					break;
		}
		if (mode == FULL)
			reset_kalman();
	}
	else
		printf("Can't change mode or wrong key.\n");
}


void handleMode (void) {
	oldroll = roll;
	oldpitch = pitch;
	oldyaw = yaw;
	oldlift = lift;
	
	switch(mode) {
		case SAFE:
			X32_leds = 1;
			safe_mode_ctrl();
			break;
		case PANIC:
			X32_leds = 2;
			panic_mode_ctrl();
			break;
		case MANUAL:
			X32_leds = 4;		
			manual_mode_ctrl();
			break;
		case CALIBRATION:
			X32_leds = 8;
			calibration_mode();
			break;
		case YAW:
			X32_leds = 16;		
			yaw_mode_ctrl();
			break;
		case FULL:
			X32_leds = 32;		
			full_mode_ctrl();
			break;
		default:
			X32_leds = 64;		
			break;
	}
}

void compute_RPMs(void) {
	//TODO clip_RPYL(roll, pitch, yaw, lift, 100);	
	clip_RPYL();
	oo1 = SCALE_AE * (lift + 2 * pitch - yaw) >> 2;
	oo2 = SCALE_AE * (lift - 2 * roll + yaw) >> 2;
	oo3 = SCALE_AE * (lift - 2 * pitch - yaw) >> 2;
	oo4 = SCALE_AE * (lift + 2 * roll + yaw) >> 2;
}

void clip_RPYL(void)
{
// top clipping
	if (roll > MAX_ROLL)
		roll = MAX_ROLL;
	if (pitch > MAX_PITCH)
		pitch = MAX_PITCH;
	if (yaw > MAX_YAW)
		yaw = MAX_YAW;
	if (lift > MAX_LIFT)
		lift = MAX_LIFT;						

// botttom clipping
	if (roll < MIN_ROLL)
		roll = MIN_ROLL;
	if (pitch < MIN_PITCH)
		pitch = MIN_PITCH;
	if (yaw < MAX_YAW)
		yaw = MIN_YAW;
	if (lift < MIN_LIFT)
		lift = MIN_LIFT;						

// rate clipping
	if((roll - oldroll) > LIMIT_RATE)
		roll = oldroll + LIMIT_RATE;
	else if((oldroll - roll) > LIMIT_RATE)
		roll = oldroll - LIMIT_RATE;

	if((pitch - oldpitch) > LIMIT_RATE)
		pitch = oldpitch + LIMIT_RATE;
	else if((oldpitch - pitch) > LIMIT_RATE)
		pitch = oldpitch - LIMIT_RATE;
		
	if((yaw - oldyaw) > LIMIT_RATE)
		yaw = oldyaw + LIMIT_RATE;
	else if((oldyaw - yaw) > LIMIT_RATE)
		yaw = oldyaw - LIMIT_RATE;
		
	if((pitch - oldlift) > LIMIT_RATE)
		lift = oldlift + LIMIT_RATE;
	else if((oldlift - lift) > LIMIT_RATE)
		lift = oldlift - LIMIT_RATE;				
}


 /*------------------------------------------------------------------
 * isr_qr_link -- QR link rx interrupt handler
 *------------------------------------------------------------------
 */
void isr_qr_link(void)
{
	/* get sensor and timestamp values */
	s0 = X32_QR_s0 - s0_bias;
	s1 = X32_QR_s1 - s1_bias; 
	s2 = X32_QR_s2 - s2_bias; 
	s3 = X32_QR_s3 - s3_bias; 
	s4 = X32_QR_s4 - s4_bias; 
	s5 = X32_QR_s5 - s5_bias;
}

void isr_timer(void)
{
//Since we are clipping the RPYL, doing the same for the motors should not be necessary but you never know!		
	if (oo1 < MIN_MOTOR1) oo1 = MIN_MOTOR1;
	if (oo1 > MAX_MOTOR1) oo1 = MAX_MOTOR1;
	
	if (oo2 < MIN_MOTOR2) oo2 = MIN_MOTOR2;
	if (oo2 > MAX_MOTOR2) oo2 = MAX_MOTOR2;
	
	if (oo3 < MIN_MOTOR3) oo3 = MIN_MOTOR3;
	if (oo3 > MAX_MOTOR3) oo3 = MAX_MOTOR3;
	
	if (oo4 < MIN_MOTOR4) oo4 = MIN_MOTOR4;
	if (oo4 > MAX_MOTOR4) oo4 = MAX_MOTOR4;
	        
	X32_QR_a0 = oo1;
	X32_QR_a1 = oo2;
	X32_QR_a2 = oo3;
	X32_QR_a3 = oo4;
}
