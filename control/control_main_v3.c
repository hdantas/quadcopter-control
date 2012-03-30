#include "console_IO.h"
#include "comm.h"
#include "serial.h"
#include "joystick.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <assert.h>

//#define JS_SENSITIVITY 512
#define KB_SHIFT 64
#define KB_MAX 127
/*lift is only positive, so its min and max value are shifted
of half range (64)*/
#define KB_MAX_LIFT 191 //127+64
#define KB_MIN_LIFT 64 //0
#define JS_SHIFT 32768

#define TRESHOLD 20000

#define JS_SLOPE -0.0128
#define JS_SLOPE_LIFT -0.004

#define JS_SENSITIVITY_ROLL 512
#define JS_SENSITIVITY_PITCH 512
#define JS_SENSITIVITY_YAW 512
#define JS_SENSITIVITY_LIFT 512

#define TIME_INTERRUPT 10000

int js_sensitivity[4] = {JS_SENSITIVITY_ROLL, JS_SENSITIVITY_PITCH, JS_SENSITIVITY_YAW, JS_SENSITIVITY_LIFT};  //sensitivity of joystick
extern int *axis;			//vector for axis values
extern char *button;			//vector for button value
int kb_RPYL[4];				//keyboard offset of roll pitch yaw lift
int js_RPYL[4];				//joystick value divided by sensitivity
int RPYL_value[4];			//js_RPYL+kb_RPYL
unsigned char RPYL_data[4];		//roll pitch yaw lift to send
int oldroll,oldpitch,oldyaw,oldlift;	//used to clipping
int quit;				//end of program

int send_fire_button;
int send_kb_button;
comm_type type_c;			//header of packets
extern int serial_handle;

/*function to get character from the serial. We use this for data loggin*/

int	rs232_getchar_nb()
{
	int 		result;
	unsigned char 	c;

	result = read(serial_handle, &c, 1);
	if (result == 0) {
		return -1;
	}
	else {
		assert(result == 1);   
		return (int) c;
	}
}

/*function to send character to the serial. We use this for start the program in the QR*/

int 	rs232_putchar(char c)
{ 
	int result;

	do {
		result = (int) write(serial_handle, &c, 1);
	} while (result == 0);   

	assert(result == 1);
	return result;
}

/*Timer interrupt. We use this function to send packets to the QR every 10 ms*/

void sigFunc(int sig) {

	if (send_kb_button) {
		if (0 != send_data(type_c, 0, 0))
			printf("Error sending buttons\n");

		send_kb_button=0;
	}

	if (send_fire_button) {
		if (0 != send_data(KEY1, 0, 0))
			printf("Error sending buttons\n");

		send_fire_button=0;
	}

	if (0 != send_data(RPYL, RPYL_data, 4))
		printf("Error sending joystick\n");
	

	/* reset the timer so we get called again in 10 ms */
	if (!quit)
		ualarm(TIME_INTERRUPT, 0);
}


/* This function map the keybord button in header for packets. Some button modify also the body of the packets
return 1 if the button is mapped in a header
return 0 if the button modify the body 
return 0 if the button is not mapped */

int source_button(char c, comm_type *type_c) 
{	
	switch (c) 
	{
		case 0x1B:		/* ESC: abort / exit */
			*type_c=KEYESC;
			return 1;
		case 0x0A:		/*increment control mode */
			*type_c=KEYRETURN;
			return 1;
		case '0':		/*Safe Mode*/
			*type_c=KEY0;
			return 1;
		case '1':		/*Panic Mode*/
			*type_c=KEY1;
			return 1;
		case '2':		/*Manual Mode*/
			*type_c=KEY2;
			return 1;
		case '3':		/*Calibration Mode*/
			*type_c=KEY3;
			return 1;
		case '4':		/*Yaw control Mode*/
			*type_c=KEY4;
			return 1;
		case '5':		/*Full control mode*/
			*type_c=KEY5;
			return 1;
		case 'a':		/* increase lift */
			*type_c=KEYA;
			if(kb_RPYL[3]<KB_MAX_LIFT)
				kb_RPYL[3]+= 1;
			return 0;
		case 'z':		/* decrease lift */
			*type_c=KEYZ;
			if(kb_RPYL[3]>KB_MIN_LIFT)
				kb_RPYL[3]-=1;
			return 0;
		case 0x43:		/*right arrow: roll down maybe*/
			*type_c=KEYRIGHT;
			if(kb_RPYL[0]<KB_MAX)
				kb_RPYL[0] += 1;
			return 0;
		case 0x44:		/*left arrow: roll up maybe*/
			*type_c=KEYLEFT;
			if(kb_RPYL[0]>0)
				kb_RPYL[0] -= 1;
			return 0;
		case 0x41:		/*up arrow: pitch down */
			*type_c=KEYUP;
			if(kb_RPYL[1]>0)
				kb_RPYL[1] -= 1;
			return 0;
		case 0x42:		/*down arrow: pitch up */
			*type_c=KEYDOWN;
			if(kb_RPYL[1]<KB_MAX)
				kb_RPYL[1] += 1;
			return 0;
		case 'q':		/* decrease yaw */
			*type_c=KEYQ;
			if(kb_RPYL[2]>0)
				kb_RPYL[2] -= 1;
			return 0;
		case 'w':		/* increase yaw */
			*type_c=KEYW;
			if(kb_RPYL[2]<KB_MAX)
				kb_RPYL[2] += 1;
			return 0;
		case 'u':		/*yaw control P up*/
			*type_c=KEYU;
			return 1;
		case 'j':		/*yaw control P down*/
			*type_c=KEYJ;
			return 1;
		case 'i':		/*roll/pitch control P1 up*/
			*type_c=KEYI;
			return 1;
		case 'k':		/*roll/pitch control P1 down*/
			*type_c=KEYK;
			return 1;
		case 'o':		/*roll/pitch control P2 up*/
			*type_c=KEYO;
			return 1;
		case 'l':		/*roll/pitch control P2 down*/
			*type_c=KEYL;
			return 1;
		default:
			return -1;	//key not map
	}
}

/*This function set a maximum variation rate for roll pitch yaw lift
Useful to avoid stall*/

void clip_RPYL(int *RPYL_data,int limit_rate)
{
	if(RPYL_data[0]>(oldroll+limit_rate))
		RPYL_data[0]=oldroll+limit_rate;
	if(RPYL_data[0]<(oldroll-limit_rate))
		RPYL_data[0]=oldroll-limit_rate;

	if(RPYL_data[1]>(oldpitch+limit_rate))
		RPYL_data[1]=oldpitch+limit_rate;
	if(RPYL_data[1]<(oldpitch-limit_rate))
		RPYL_data[1]=oldpitch-limit_rate;

	if(RPYL_data[2]>(oldyaw+limit_rate))
		RPYL_data[2]=oldyaw+limit_rate;
	if(RPYL_data[2]<(oldyaw-limit_rate))
		RPYL_data[2]=oldyaw-limit_rate;

	if(RPYL_data[3]>(oldlift+limit_rate))
		RPYL_data[3]=oldlift+limit_rate;
	if(RPYL_data[3]<(oldlift-limit_rate))
		RPYL_data[3]=oldlift-limit_rate;
}
			


int main(void) {
	//Declare variables
	comm_type type;
	unsigned char* data;
	int i;
	int len;
	char c;
	
	oldroll=oldpitch=oldyaw=oldlift=0;

	quit=0;
	
	//we send only positive number so the zero is in KB_SHIFT (64)
	kb_RPYL[0]=kb_RPYL[1]=kb_RPYL[2]=kb_RPYL[3]=KB_SHIFT;

	for (i=0;i<4;i++)
		js_RPYL[i]=0;//TODO
	
	
	//Initialise communication
	if (0 != comm_init())
		return -1;

	term_initio();

	//joy_open();

	 /* set up our signal handler to catch SIGALRM */
  	signal(SIGALRM, sigFunc);

  	/* start the timer - we want to send packets every 10 ms */
  	ualarm(TIME_INTERRUPT, 0);

	/* discard any incoming text */
	while ((c = rs232_getchar_nb()) != -1)
	fputc(c,stderr);

	//start the program on QR

	rs232_putchar('s');
	rs232_putchar('\n');

	while(!quit) {

		if ((c = term_getchar_nb()) != -1) 
		{
			//press a key button	
			if (source_button(c, &type_c)==1)
				send_kb_button=1;

		}
		
	
		/*if(read_joy()!= 0) 
			{
				// calculate the sensitivity to improve the joystick movement
				js_sensitivity[0]=JS_SLOPE*(abs(axis[0]))+JS_SENSITIVITY_ROLL;
				js_sensitivity[1]=JS_SLOPE*(abs(axis[1]))+JS_SENSITIVITY_PITCH;
				js_sensitivity[2]=JS_SLOPE*(abs(axis[2]))+JS_SENSITIVITY_YAW;
				js_sensitivity[3]=JS_SLOPE_LIFT*(JS_SHIFT-axis[3])+JS_SENSITIVITY_LIFT;

				// divide the joystick values by sensitivity 		
				js_RPYL[0] = axis[0] / js_sensitivity[0];
				js_RPYL[1] = axis[1] / js_sensitivity[1];
				js_RPYL[2] = axis[2] / js_sensitivity[2];
				js_RPYL[3] = -axis[3]/ js_sensitivity[3];
				
				//press fire button
				if (button[0]==1)				
					send_fire_button=1;					
			}*/


		/* sum joystick value with keyboard offset */		
		
		RPYL_value[0]=js_RPYL[0]+kb_RPYL[0];
		RPYL_value[1]=js_RPYL[1]+kb_RPYL[1];
		RPYL_value[2]=js_RPYL[2]+kb_RPYL[2];
		RPYL_value[3]=js_RPYL[3]+kb_RPYL[3];

		/* clip max and minimum value for joystick */
		
		if (RPYL_value[0] <= 0) RPYL_value[0] = 0;
		if (RPYL_value[0] >= KB_MAX) RPYL_value[0] = KB_MAX;
		if (RPYL_value[1] <= 0) RPYL_value[1] = 0;
		if (RPYL_value[1] >= KB_MAX) RPYL_value[1] = KB_MAX;
		if (RPYL_value[2] <= 0) RPYL_value[2] = 0;
		if (RPYL_value[2] >= KB_MAX) RPYL_value[2] = KB_MAX;
		if (RPYL_value[3] <= 0) RPYL_value[3] = 0;
		if (RPYL_value[3] >= KB_MAX) RPYL_value[3] = KB_MAX;

		/* convert joystick value in char */

		for (i=0;i<4;i++)
			RPYL_data[i]=(unsigned char) RPYL_value[i];

		/* limit the variation of roll pitch yaw lift*/

		clip_RPYL(RPYL_value,5);

		/* I save the old values, because I use them in "clip_RPYL" function */
			
		oldroll=RPYL_data[0];
		oldpitch=RPYL_data[0];
		oldyaw=RPYL_data[0];
		oldlift=RPYL_data[0];

		/* Visualization real time */

		if ((c = rs232_getchar_nb()) != -1) {
			if(c==7) //I recive all the data, refresh screen
				system("clear");
			else if(c==11){ //QR has finished to execute the program, we can abort the program
				quit=1;
			}
			else{		//is a character of data loggin, write on terminal
				term_putchar(c);
				fflush(stdout);
			}
	
			
		}


	}

	printf("\nExiting...\n");
	fflush(stderr);

	term_exitio();
	//joy_close();
	//Uninitialise
	comm_uninit();
	system("clear");
	return 0;
}
