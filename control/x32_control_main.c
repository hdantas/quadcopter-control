#include <stdio.h>
#include <stdlib.h>

#include "x32_common.h"
#include "comm.h"
#include "convert.h"
#include "x32_modes.h"
#include "filter.h"
#include "log.h"
#include "x32_log.h"


int lift, roll, pitch, yaw;		//value received from the serial
int oldoo1, oldoo2, oldoo3, oldoo4;	//old values of engine, usefull to avoid stall
int lift_error, roll_error, pitch_error, yaw_error;	//signals of the control
comm_type mode;		//mode of the QR
comm_type type;		//header of packets
volatile int finished;	//end of program

int flag_data_loggin;	//data loggin is ready
int flag_feedback;	//ready to send feedback to PC

unsigned int time_last_packet;	//used to check pc link
int counter_pc_link;	//used like multiple of timer
unsigned int isr_qr_time;	//used to check qr link
unsigned int start_time;	//used to measure the latency
unsigned int control_latency_time;	//latency time

int oo1, oo2, oo3, oo4;			//engine values
int s0, s1, s2, s3, s4, s5, timestamp;	//sensors values
int s0_bias, s1_bias, s2_bias, s3_bias, s4_bias, s5_bias;	//sensors bias
int p_yaw;	//proportional control value of yaw control
int p1_full, p2_full;	//proportional controls value of full control

unsigned char* data;	//body of packets
int len;		//lenght of packets

int datalogbuffer[16];	//Array to efficiently add values to log
int tempvalue;
unsigned char* feedbackbuffer; //Buffer to convert integers for transmission
int feedbacklen;

void main(void) {

	init_state();

	//Initialise communication
	if (0 != comm_init())
		return;
		
	//Start logging
	log_start();

	ENABLE_INTERRUPT(INTERRUPT_GLOBAL);

	//Wait for handshake
	X32_display = 0x1111;
	while (0 == recv_data(&type, &data, &len));
	X32_display = 0x0000;
	if (type != HANDSHAKE)
		//Stop immediately
		finished = 1;
	else
		//Ready to start
		finished = 0;


	SET_INTERRUPT_VECTOR(INTERRUPT_XUFO, &isr_qr_link);
	SET_INTERRUPT_PRIORITY(INTERRUPT_XUFO, 21);
	ENABLE_INTERRUPT(INTERRUPT_XUFO);

	// prepare timer interrupt for 1000 Hz engine control
	X32_timer_per = 1 * CLOCKS_PER_MS;         
	SET_INTERRUPT_VECTOR(INTERRUPT_TIMER1, &isr_timer);
	SET_INTERRUPT_PRIORITY(INTERRUPT_TIMER1, 10);
	ENABLE_INTERRUPT(INTERRUPT_TIMER1);

	SET_INTERRUPT_VECTOR(INTERRUPT_OVERFLOW, &isr_overflow);
	SET_INTERRUPT_PRIORITY(INTERRUPT_OVERFLOW, 40);

	SET_INTERRUPT_VECTOR(INTERRUPT_OUT_OF_MEMORY, &isr_out_of_memory);
	SET_INTERRUPT_PRIORITY(INTERRUPT_OUT_OF_MEMORY, 40);

	ENABLE_INTERRUPT(INTERRUPT_OUT_OF_MEMORY);




	while (!finished)
	{	
		/*check pc and qr link*/
		check_pc_link();
		check_qr_link();

		/*take data from the serial*/
		if ( recv_data(&type, &data, &len) == 1) {
			/*store the time of the last packet received, we use this value to check the pc link*/
			time_last_packet=X32_us_clock;
			/*elaboration of the data received*/
			handleInput();
			free(data);
		}
		
		//Display mode
		X32_display = mode-SAFE;

		/*Log data*/
		if (flag_data_loggin == 1){
			//Fill temporary data structure
			datalogbuffer[0] = mode - SAFE;
			datalogbuffer[1] = roll;
			datalogbuffer[2] = pitch;
			datalogbuffer[3] = yaw;
			datalogbuffer[4] = lift;
			datalogbuffer[5] = oo1;
			datalogbuffer[6] = oo2;
			datalogbuffer[7] = oo3;
			datalogbuffer[8] = oo4;
			datalogbuffer[9] = s0;
			datalogbuffer[10] = s1;
			datalogbuffer[11] = s2;
			datalogbuffer[12] = s3;
			datalogbuffer[13] = s4;
			datalogbuffer[14] = s5;
			datalogbuffer[15] = control_latency_time;
			//Write to log
			log_data(LOG_VALUES, (unsigned char*) datalogbuffer, 16*4);
			//Reset log flag
			flag_data_loggin=0;
		}
		
		//Send feedback to PC if flag is odd
		if (flag_feedback & 0x01) {
		
			/* Critical section */
			DISABLE_INTERRUPT(INTERRUPT_GLOBAL);
			
			if (flag_feedback == 1)
				tempvalue = mode - SAFE;
			else if (flag_feedback == 3)
				tempvalue = roll;
			else if (flag_feedback == 5)
				tempvalue = pitch;
			else if (flag_feedback == 7)
				tempvalue = yaw;
			else if (flag_feedback == 9)
				tempvalue = lift;
			else if (flag_feedback == 11)
				tempvalue = oo1;
			else if (flag_feedback == 13)
				tempvalue = oo2;
			else if (flag_feedback == 15)
				tempvalue = oo3;
			else if (flag_feedback == 17)
				tempvalue = oo4;
			else if (flag_feedback == 19)
				tempvalue = s0;
			else if (flag_feedback == 21)
				tempvalue = s1;
			else if (flag_feedback == 23)
				tempvalue = s2;
			else if (flag_feedback == 25)
				tempvalue = s3;
			else if (flag_feedback == 27)
				tempvalue = s4;
			else if (flag_feedback == 29)
				tempvalue = s5;
			else if (flag_feedback == 31)
				tempvalue = p_yaw;
			else if (flag_feedback == 33)
				tempvalue = p1_full;
			else if (flag_feedback == 35)
				//tempvalue = p2_full;
				tempvalue = 25;
			else if (flag_feedback == 37)
				tempvalue = control_latency_time;
			
			//Create a sendable data structure
			make_int_sendable(tempvalue, &feedbackbuffer, &feedbacklen);
			//Add type (ugly)
			feedbackbuffer[5] = flag_feedback;
			
			/* End of critical section */
			ENABLE_INTERRUPT(INTERRUPT_GLOBAL);
			
			//Send the data
			send_data(FEEDBACK_DATA, feedbackbuffer, 8);
			
			//Free buffer
			free(feedbackbuffer);
			
			//Reset flag
			flag_feedback++;
			if (flag_feedback >= 38)
				flag_feedback = 0;
		}

	}
	/*reserved character for stop the program on PC*/
//	printf("%c", 11);
	
	//Secret handshake to notify PC of quitting
	send_data(KEYESC, 0, 0);

	//Uninitialise
	X32_leds=0;
	comm_uninit();
}

// Initialization of all the variable

void init_state(void)
{
	lift = roll = pitch = yaw = 0;
	oo1 = oo2 = oo3 = oo4 = 0;
	p_yaw = INITIAL_P_YAW;
	p1_full = INITIAL_P1_FULL;
	p2_full = INITIAL_P2_FULL;		
	s0 = s1 = s2 = s3 = s4 = s5 = 0;
	s0_bias = s1_bias = s2_bias = s3_bias = s4_bias = s5_bias = 0;
	mode = INITIAL_MODE; //starts in INITIAL_STATE mode
	init_kalman();
	init_butterworth();
	time_last_packet=0;
	counter_pc_link=0;
	isr_qr_time=0;
	flag_data_loggin=0;
	flag_feedback = 0;
	calibration_mode();
}

//elaboration of received data

void handleInput (void) {
	if (type == KEY1) {	//request to go on panic mode
		mode = PANIC;
		return;
	}
	//We send only positive values, so we must shift the data
	if (type == RPYL) {
		roll = data[0] - 64;
		pitch = data[1] - 64;
		yaw = data[2] - 64;
		lift = data[3];
		return;
	}

	//Control Parameter changes
	
	if (type >= KEYY && type <= KEYL){
		switch (type) {
			case KEYY: ////increase P2PHI
				p2phi += UP_P2PHI;
				if (p2phi > MAX_P2PHI) p2phi = MAX_P2PHI;
				break;
			case KEYH: //decrease P2PHI
				p2phi -= DOWN_P2PHI;
				if (p2phi < MIN_P2PHI) p2phi = MIN_P2PHI;
				break;			
			case KEYU: /*yaw control P up*/
				p_yaw += UP_P_YAW;
				if (p_yaw > MAX_P_YAW) p_yaw = MAX_P_YAW;
				break;
			case KEYJ: /*yaw control P down*/
				p_yaw -= DOWN_P_YAW;
				if (p_yaw < MIN_P_YAW) p_yaw = MIN_P_YAW;
				break;
			case KEYI: //roll/pitch control P1 up
				p1_full += UP_P1_FULL;
				if (p1_full > MAX_P1_FULL) p1_full = MAX_P1_FULL;
				break;
			case KEYK: //roll/pitch control P1 down
				p1_full -= DOWN_P1_FULL;
				if (p1_full < MIN_P1_FULL) p1_full = MIN_P1_FULL;
				break;
			case KEYO: //roll/pitch control P2 up
				p2_full += UP_P2_FULL;
				if (p2_full > MAX_P2_FULL) p2_full = MAX_P2_FULL;				
				break;
			case KEYL: //roll/pitch control P2 down
				p2_full -= DOWN_P2_FULL;
				if (p2_full < MIN_P2_FULL) p2_full = MIN_P2_FULL;
				break;
			default:
				break;				
		}
		return;
	}

	//Check for engines stopped
	if ((oo1==0) && (oo2==0) && (oo3==0) && (oo4==0)) {
		if (type == KEYESC) {
			//Terminate
			finished=1;
			mode=SAFE;				
		} else if (type == REQ_LOG) {
			//Disable all interrupts that result in traffic
			DISABLE_INTERRUPT(INTERRUPT_TIMER1);
			//Send log
			log_transmit();
			//Reenable interrupts
			ENABLE_INTERRUPT(INTERRUPT_TIMER1);
		}
	} else {
		if (type == REQ_LOG) {
			//Not clear to send
			send_data(UNAVAILABLE, 0, 0);
		}
	}
		
	//Change control
	//only if the set points are zero and the output of engine are null
	if ((oo1==0) && (oo2==0) && (oo3==0) && (oo4==0) && (lift==0) && (roll==0) && (pitch==0) && (yaw==0))
	{		
	  	switch (type) {			
					
			case KEYRETURN: /*increment control mode */
				if (mode != FULL)
					mode++;
				else
					mode=SAFE;				
				break;	
			case KEY0: /*Safe Mode*/
				mode=SAFE;
				break;
			case KEY2: /*Manual Mode*/
				mode=MANUAL;
				break;
			case KEY3: /*Calibration Mode*/
				mode=CALIBRATION;
				break;					
			case KEY4: /*Yaw control Mode*/
				mode=YAW;
				break;
			case KEY5: /*Full control mode*/
				mode=FULL;
				break;
			default:
				break;
		}

		/*reset filters parameters*/
		if (mode == FULL){
			reset_kalman();
			init_butterworth();
		}

		if (mode == YAW)
			init_butterworth();
	}

}

//this function choose the control

void handleMode (void) {

//we store the old engine value and we use them to avoid stall
	oldoo1 = oo1;
	oldoo2 = oo2;
	oldoo3 = oo3;
	oldoo4 = oo4;
	
	switch(mode) {
		case SAFE:
			X32_leds =(X32_leds & 15) | 32;	// (X32_leds & 0000 1111) | 0010 0000
			safe_mode_ctrl();
			break;
		case PANIC:
			X32_leds =(X32_leds & 15) | 64;	//(X32_leds & 0000 1111) | 0100 0000
			panic_mode_ctrl();
			break;
		case MANUAL:
			X32_leds =(X32_leds & 15) | 96; //(X32_leds & 0000 1111) | 0110 0000
			manual_mode_ctrl();
			break;
		case CALIBRATION:
			X32_leds =(X32_leds & 15) | 128; //(X32_leds & 0000 1111) | 1000 0000
			calibration_mode();
			break;
		case YAW:
			X32_leds =(X32_leds & 15) | 160; //(X32_leds & 0000 1111) | 1010 0000	
			yaw_mode_ctrl();
			break;
		case FULL:
			X32_leds =(X32_leds & 15) | 192; //(X32_leds & 0000 1111) | 1100 0000
			full_mode_ctrl();
			break;
		default:
			X32_leds =(X32_leds & 15) | 224; //(X32_leds & 0000 1111) | 1110 0000		
			break;
	}
}


/*QR link rx interrupt handler */

void isr_qr_link(void)
{
	/* get sensor values */
	s0 = X32_QR_s0 - s0_bias;
	s1 = -(X32_QR_s1 - s1_bias); 
	s2 = X32_QR_s2 - s2_bias; 
	s3 =-(X32_QR_s3 - s3_bias); 
	s4 =-(X32_QR_s4 - s4_bias); 
	s5 = - (X32_QR_s5 - s5_bias);

	/* we use this time to check the qr link*/
	isr_qr_time=X32_QR_timestamp;
}

//interrupt for 1000 Hz engine control

void isr_timer(void)
{
	//measure the latency of control
	start_time=X32_us_clock;

	//we use this like multiple of timer
	counter_pc_link++;

	//control
	handleMode();

	//clip the engine value
	clip_AE();

	//usefull when you are the QR on your hands and there is some control	
	if (lift == 0){
		oo1 = 0;
		oo2 = 0;
		oo3 = 0;
		oo4 = 0;
	}

	//set engine values		

	X32_QR_a0 = oo1;
	X32_QR_a1 = oo2;
	X32_QR_a2 = oo3;
	X32_QR_a3 = oo4;

	//blink led if the program is running

	if(counter_pc_link==BLINK_COUNT)
		if(!finished) {
			toggle_led(0);
			counter_pc_link=0;
		}

	//store the control latency

	control_latency_time=X32_us_clock-start_time;

	//every 100 ms we set the flag on for data loggin (10 Hz)

	if (X32_ms_clock % 100 == 0) {
		//Log
		flag_data_loggin=1;
	}
	
	//every 10ms we set the flag to send feedback to PC (100Hz)
	if (X32_ms_clock % 10 == 0) {
		//Feedback
		flag_feedback++;
		if (flag_feedback >= 38)
			flag_feedback = 0;
	}
	
}

//overflow

void isr_overflow(void)
{
	toggle_led(7);
	mode=PANIC;
}

//out of memory

void isr_out_of_memory(void)
{
	//log and flag error
	X32_leds |= 0x20;
	//printf("Out of memory\r");
	if (mode != SAFE)
		mode = PANIC;
}


//check pc link

void check_pc_link(void)
{
	if (time_last_packet!=0) {
		if (X32_us_clock-time_last_packet>TIME_OUT_LINK) {
			if (mode != SAFE)
				mode=PANIC;
			X32_leds&=253; //1111 1101
		}
		else
			X32_leds|=2;
		
	}
}

//check qr link

void check_qr_link(void)
{
	if (isr_qr_time!=0) {
		if (X32_us_clock-isr_qr_time>TIME_OUT_QR_INT) {
			//mode=PANIC;
			X32_leds&=251; //1111 1011
		}
		else
			X32_leds|=4;
			
	}
}

void clip_AE()
{

	//clip max and min values	

	if((mode==YAW) || (mode==FULL) || (mode==MANUAL)) { //in calibration and safe engine values are null

		if (oo1 < MIN_MOTOR1) oo1 = MIN_MOTOR1;
		if (oo1 > MAX_MOTOR1) oo1 = MAX_MOTOR1;
	
		if (oo2 < MIN_MOTOR2) oo2 = MIN_MOTOR2;
		if (oo2 > MAX_MOTOR2) oo2 = MAX_MOTOR2;
	
		if (oo3 < MIN_MOTOR3) oo3 = MIN_MOTOR3;
		if (oo3 > MAX_MOTOR3) oo3 = MAX_MOTOR3;
	
		if (oo4 < MIN_MOTOR4) oo4 = MIN_MOTOR4;
		if (oo4 > MAX_MOTOR4) oo4 = MAX_MOTOR4;
	}				

	// rate clipping
	if(oo1 - oldoo1 > LIMIT_RATE)
		oo1 = oldoo1 + LIMIT_RATE;
	else if(oldoo1 - oo1 > LIMIT_RATE)
		oo1 = oldoo1 - LIMIT_RATE;

	if(oo2 - oldoo2 > LIMIT_RATE)
		oo2 = oldoo2 + LIMIT_RATE;
	else if(oldoo2 - oo2 > LIMIT_RATE)
		oo2 = oldoo2 - LIMIT_RATE;
		
	if(oo3 - oldoo3 > LIMIT_RATE)
		oo3 = oldoo3 + LIMIT_RATE;
	else if(oldoo3 - oo3 > LIMIT_RATE)
		oo3 = oldoo3 - LIMIT_RATE;
		
	if(oo4 - oldoo4 > LIMIT_RATE)
		oo4 = oldoo4 + LIMIT_RATE;
	else if(oldoo4 - oo4 > LIMIT_RATE)
		oo4 = oldoo4 - LIMIT_RATE;

}

//toggle led

void toggle_led(int i) 
{
	X32_leds = (X32_leds ^ (1 << i));
}

