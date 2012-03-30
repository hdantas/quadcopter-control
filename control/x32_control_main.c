#include <stdio.h>
#include <stdlib.h>

#include "x32_common.h"
#include "comm.h"
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

void main(void) {

	init_state();
	 
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

	//Initialise communication
	if (0 != comm_init())
		return;

	ENABLE_INTERRUPT(INTERRUPT_GLOBAL);
	ENABLE_INTERRUPT(INTERRUPT_OUT_OF_MEMORY);

	finished=0;


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

		/*send data loggin on PC*/
		if (flag_data_loggin == 1){
			/*printf("%c", 7);	//reserved character for refreshing the screen
			printf("mode:  %1d\n\n", mode-26);
			printf("roll:  %4d\tpitch: %4d\tyaw:   %4d\tlift:  %4d\n\n",roll, pitch, yaw, lift);
			printf("oo1:   %4d\too2:   %4d\too3:   %4d\too4:   %4d\n\n",oo1, oo2, oo3, oo4);
			printf("ax:    %4d\tay:    %4d\taz:    %4d\n\n",s1, s0, s2);
			printf("gyrox: %4d\tgyroy: %4d\tgyroz: %4d\n\n", s3, s4, s5);
			printf("control latency: %5d", control_latency_time);
			flag_data_loggin=0;*/
		}

	}
	/*reserved character for stop the program on PC*/
//	printf("%c", 11);

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

	
	//Change control
	//only if the set points are zero and the output of engine are null
	if ((oo1==0) && (oo2==0) && (oo3==0) && (oo4==0) && (lift==0) && (roll==0) && (pitch==0) && (yaw==0))
	{		
	  	switch (type) {			
				case KEYESC: /* ESC: abort / exit */				
					finished=1;
					mode=SAFE;				
					break;		
				case KEYRETURN: /*increment control mode */
					if (mode != FULL)
						mode++;
					else
						mode=SAFE;				
					break;
				case REQ_LOG: /*Request Logfile */
					log_transmit();
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
		if (flag_data_loggin == 0)
			flag_data_loggin=1;
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
	mode = PANIC;
}


//check pc link

void check_pc_link(void)
{
	if (time_last_packet!=0) {
		if (X32_us_clock-time_last_packet>TIME_OUT_LINK) {
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

