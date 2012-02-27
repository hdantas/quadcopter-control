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

typedef enum {
		SAFE,
		PANIC,
		MANUAL,
		CALIBRATION,
		YAW,
		FULL
} ctr_mode;

int finished;

int kb_lift, kb_yaw, kb_pitch, kb_roll;
int js_lift, js_yaw, js_pitch, js_roll;
int lift, roll, pitch, yaw;
int oo1, oo2, oo3, oo4;
ctr_mode mode;

void isr_timer(void);
void safe_mode_ctrl(comm_type type, unsigned char* data, int len);
void panic_mode_ctrl(comm_type type, unsigned char* data, int len);
void manual_mode_ctrl(comm_type type, unsigned char* data, int len);
void calibration_mode(comm_type type, unsigned char* data, int len);
void yaw_mode_ctrl(comm_type type, unsigned char* data, int len);
void full_mode_ctrl(comm_type type, unsigned char* data, int len);
void keyb(comm_type type);
void init_state(void);

void main(void) {
	//Declare variables
	comm_type type;
	unsigned char* data;
	int len;
	int I;
	int result=0;

	/* prepare QR rx interrupt handler and associated variables
	 */
/*        SET_INTERRUPT_VECTOR(INTERRUPT_XUFO, &isr_qr_link);*/
/*        SET_INTERRUPT_PRIORITY(INTERRUPT_XUFO, 21);*/
/*        ENABLE_INTERRUPT(INTERRUPT_XUFO);*/

	/*An interrupt is raised (INTERRUPT_XUFO) once the 7 new values are*/
	/*available to the X32. As an additional feature, there is also a counter register (PERIPHERAL_XUFO_COUNT) that*/
	/*holds the number of times a 7-value frame has been received (every 1/(1,270) ≈ 0.7 ms). In order to facilitate*/
	/*accurate sensor data logging, the counter register PERIPHERAL_XUFO_TIMESTAMP holds the 32 bit timestamp of*/
	/*the last received sensor data frame (in units of 20 ns, based on the X32’s 50 MHz clock).*/
	/*number of sensors 6 + state battery = 7 values*/
 
        /* prepare timer interrupt for 500 Hz engine control (i.e., 5 ms)
         */
        X32_timer_per = 2 * CLOCKS_PER_MS;
        SET_INTERRUPT_VECTOR(INTERRUPT_TIMER1, &isr_timer);
        SET_INTERRUPT_PRIORITY(INTERRUPT_TIMER1, 22);
        ENABLE_INTERRUPT(INTERRUPT_TIMER1);

	init_state();

	//Initialise communication
	if (0 != comm_init())
		return;

	ENABLE_INTERRUPT(INTERRUPT_GLOBAL);

	//Initialise
	mode=SAFE;
	finished=0;	
//	printf("Mode is: %d\n", mode);
//	printf("\nQR is Ready to receive data...\n");

//	log_msg("QR is ready to receive data...");
//	log_int(mode);

	X32_leds |= 0x01;

	//Wait until data can be received
	while (!finished)
	{
		X32_leds |= 0x04;
		result = recv_data(&type, &data, &len);
//		printf("%d\n", result);
		if(result == 1) 
		{		
		
			switch (mode) 
			{			
				case SAFE: /* Safe Mode*/
					//printf("%d, %2x, %d\n", type, data[0], len);
					safe_mode_ctrl(type, data, len);
					break;
				case PANIC: /* Panic Mode*/
					panic_mode_ctrl(type, data, len);
					break;
				case MANUAL: /* Manual Mode*/	
					manual_mode_ctrl(type, data, len);
					break;
				case CALIBRATION: /* Calibration Mode*/
					calibration_mode(type, data, len);
					break;
				case YAW: /* Yaw control Mode*/
					yaw_mode_ctrl(type, data, len);
					break;
				case FULL: /* Full control Mode*/
					full_mode_ctrl(type, data, len);		
				default:
					break;
    			}
		
			//Free buffer memory again (IMPORTANT!)
			free(data);
		} else if (result == -1) {
			X32_leds |= 0x20;
		}
		
	}

	X32_leds |= 0x10;

	//Uninitialise
	comm_uninit();
}

void safe_mode_ctrl(comm_type type, unsigned char* data, int len)
	{
		if (type==KEY_ESC)
		{
			if ((roll==0) && (pitch==0) && (yaw==0) && (lift==0))
			{		
				printf("Exiting...\n");			
				finished=1;
			}
			else 
				printf("Error roll, pitch, yaw and lift not zero, can't esc\n");	
					
		}
		else if (type==KEY_Q)//TODO only "2" key
		{
			if ((roll==0) && (pitch==0) && (yaw==0) && (lift==0))
			{
				mode=MANUAL;
			}
			else 
				printf("Error roll, pitch, yaw and lift not zero, can't switch\n");
		}
	}
void panic_mode_ctrl(comm_type type, unsigned char* data, int len)
	{
	}
void manual_mode_ctrl(comm_type type, unsigned char* data, int len)
	{
		if (len==0)	//data key
		{
			if ((type!=KEY_Q)&&(type!=KEY_Q)&&(type!=KEY_Q)&&(type!=KEY_Q)) //TODO		
			{				
				keyb(type);
			}
			else
			{
				if ((roll==0) && (pitch==0) && (yaw==0) && (lift==0))
				{				
					keyb(type);
					//printf("W\n");
				}
			}
		}
		else if (len==4)//axes data
		{
			js_roll=data[0];
			js_pitch=data[1];
			js_yaw=data[2];
			js_lift=data[3];
		}
		
		roll = kb_roll + js_roll;
		pitch = kb_pitch + js_pitch;
		yaw = kb_yaw + js_yaw;
		lift = kb_lift + js_lift;
		if (lift < 0) lift = 0;
		if (lift>128) lift=128;
		if (roll>64) roll=64;
		if (roll<-64) roll=-64;
		if (yaw>64) yaw=64;
		if (yaw<-64) yaw=-64;
		
		oo1 = (lift + 2 * pitch - yaw) / 4;
		oo2 = (lift - 2 * roll + yaw) / 4;
		oo3 = (lift - 2 * pitch - yaw) / 4;
		oo4 = (lift + 2 * roll + yaw) / 4;
	}
void calibration_mode(comm_type type, unsigned char* data, int len)
	{
	}
void yaw_mode_ctrl(comm_type type, unsigned char* data, int len)
	{
	}
void full_mode_ctrl(comm_type type, unsigned char* data, int len)
	{
	}

void 	keyb(comm_type type)
{
  	switch (type) {

		//TODO values of case are not right
			
		case 0x1B: /* ESC: abort / exit */
			break;
		case 0x0A: /*increment control mode */
			break;

		/* choose mode*/

		case '0': /*Safe Mode*/
			mode=SAFE;
			break;
		case '1': /*Panic Mode*/
			mode=PANIC;				
			break;
		case '2': /*Manual Mode*/
			mode=MANUAL;
			break;
		case '3': /*Calibration Mode*/
			mode=CALIBRATION;
			break;
		case '4': /*Yaw control Mode*/
			mode=YAW;
			break;
		case '5': /*Full control mode*/
			mode=FULL;
			break;

		/* quad rotor control*/	

		case 'a': /* increase lift */
			kb_lift += 1;
      			break;
		case 'z': /* decrease lift */
			kb_lift -= 1;
      			break;
		case 0x43: /*right arrow: roll down maybe*/
			kb_roll += 1;
			break;
		case 0x44: /*left arrow: roll up maybe*/
			kb_roll -= 1;
			break;
		case 0x41: /*up arrow: pitch down */
			kb_pitch -= 1;
			break;
		case 0x42: /*down arrow: pitch up */
			kb_pitch += 1;
			break;
		case 'w': /* increase yaw */
			kb_yaw += 1; 
      			break;
		case 'q': /* decrease yaw */
			kb_yaw -= 1; 
      			break;
		case 'u': /*yaw control P up*/
			break;
		case 'j': /*yaw control P down*/
			break;
		case 'i': /*roll/pitch control P1 up*/
			break;
		case 'k': /*roll/pitch control P1 down*/
			break;
		case 'o': /*roll/pitch control P2 up*/
			break;
		case 'l': /*roll/pitch control P2 down*/
			break;
		default:
			break;
    	}
}

void init_state(void)
{
	kb_lift=kb_yaw=kb_pitch=kb_roll=0;
	js_lift=js_yaw=js_pitch=js_roll=0;
	lift=roll=pitch=yaw=0;
	oo1=oo2=oo3=oo4=0;
}

void isr_timer(void)
{
	
        /* send actuator values to ae0..3 QR peripheral regs
         */
        X32_QR_a0 = oo1;
        X32_QR_a1 = oo2;
        X32_QR_a2 = oo3;
        X32_QR_a3 = oo4;
}

