#include "console_IO.h"
#include "comm.h"
#include "joystick.h"
#include "log.h"
#include "log_pc.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

//#define JS_SENSITIVITY 512
#define KB_SHIFT 64
#define KB_MAX 127
#define KB_MAX_LIFT 191
#define JS_SHIFT 32768
#define RPYL_SHIFT 0

#define TRESHOLD 20000

#define JS_SLOPE -0.0128
#define JS_SLOPE_LIFT -0.004

#define JS_SENSITIVITY_ROLL 512
#define JS_SENSITIVITY_PITCH 512
#define JS_SENSITIVITY_YAW 512
#define JS_SENSITIVITY_LIFT 512

/*#define JS_SENSITIVITY_CEILING_ROLL 256*/
/*#define JS_SENSITIVITY_CEILING_PITCH 256*/
/*#define JS_SENSITIVITY_CEILING_YAW 256*/
/*#define JS_SENSITIVITY_CEILING_LIFT 256*/
#define TIME_INTERRUPT 10000

int js_sensitivity[4] = {JS_SENSITIVITY_ROLL, JS_SENSITIVITY_PITCH, JS_SENSITIVITY_YAW, JS_SENSITIVITY_LIFT};
extern int *axis;		//vector for axes values
extern char *button;		//vector for button value
int kb_RPYL[4];
int js_RPYL[4];
int RPYL_value[4];
unsigned char RPYL_data[4];
int oldroll,oldpitch,oldyaw,oldlift;

int send_fire_button;
int send_kb_button;
comm_type type_c;

int feedbackvalues[19];
int feedbacktype;

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
	ualarm(TIME_INTERRUPT, 0);
}


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
			printf("Entering safe mode\n");
			return 1;
		case '1':		/*Panic Mode*/
			printf("Entering panic mode\n");
			*type_c=KEY1;
			return 1;
		case '2':		/*Manual Mode*/
			printf("Entering manual mode\n");
			*type_c=KEY2;
			return 1;
		case '3':		/*Calibration Mode*/
			printf("Entering calibration mode\n");
			*type_c=KEY3;
			return 1;
		case '4':		/*Yaw control Mode*/
			printf("Entering yaw mode\n");
			*type_c=KEY4;
			return 1;
		case '5':		/*Full control mode*/
			printf("Entering full mode\n");
			*type_c=KEY5;
			return 1;
		case 'a':		/* increase lift */
			printf("Increase lift mode\n");
			*type_c=KEYA;
			if(kb_RPYL[3]<KB_MAX_LIFT)
				kb_RPYL[3]+= 1;
			return 0;
		case 'z':		/* decrease lift */
			printf("Decrease lift mode\n");
			*type_c=KEYZ;
			if(kb_RPYL[3]>KB_SHIFT)
				kb_RPYL[3]-=1;
			return 0;
		case 0x44:	/*left arrow: roll up*/
			*type_c=KEYRIGHT;
			if(kb_RPYL[0]<KB_MAX)
				kb_RPYL[0] += 1;
			return 0;
		case 0x43:	/*right arrow: roll down*/
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
			printf("Increase yaw control P\n");
			*type_c=KEYU;
			return 1;
		case 'j':		/*yaw control P down*/
			printf("Decrease yaw control P\n");
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
		case 'r':		/*retrieve logfile*/
			*type_c=REQ_LOG;
			//Stop timer to make comm blocking
		  	ualarm(0, 0);
		  	printf("Retrieving log...\n");
			if (0 == retrieve_log())
				printf("Successfully acquired logfile.\n");
			else
				printf("Unable to retrieve logfile.\n");
			// Restart timer
			signal(SIGALRM, sigFunc);
			ualarm(TIME_INTERRUPT, 0);
			return 0;
		default:
			printf("key not mapped error\n");
			return -1;	//key not map
	}
}

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
	int quit;
	char c;
	
	oldroll = oldpitch = oldyaw = oldlift = 0;

	quit = 0;

	kb_RPYL[0] = kb_RPYL[1] = kb_RPYL[2] = kb_RPYL[3] = KB_SHIFT;

	for (i=0;i<4;i++)
		js_RPYL[i]=0;
		
	//Initialize feedback data
	for (i=0;i<19;i++)
		feedbackvalues[i] = 0;
	
	
	//Initialise communication
	if (0 != comm_init())
		return -1;

	term_initio();

//	joy_open();

	 /* set up our signal handler to catch SIGALRM */
  	signal(SIGALRM, sigFunc);

  	/* start the timer - we want to send packets every 10 ms */
  	ualarm(TIME_INTERRUPT, 0);
  	
  	//Send handshake
  	send_data(HANDSHAKE, 0, 0);
	
	
			

	//TODO:remove
	//Initialize
	js_RPYL[0] = 0;
	js_RPYL[1] = 0;
	js_RPYL[2] = 0;
	js_RPYL[3] = 0;

	kb_RPYL[0] = 0;
	kb_RPYL[1] = 0;
	kb_RPYL[2] = 0;
	kb_RPYL[3] = 0;

	while(!quit) {

		if ((c = term_getchar_nb()) != -1) 
		{
			//press a key button	
			if (source_button(c, &type_c)==1)
				send_kb_button=1;

		}
		
	
/*		if(read_joy()!= 0) 
			{
				//printf("%d\n", axis[2]);

				js_sensitivity[0]=JS_SLOPE*(abs(axis[0]))+JS_SENSITIVITY_ROLL;
				js_sensitivity[1]=JS_SLOPE*(abs(axis[1]))+JS_SENSITIVITY_PITCH;
				js_sensitivity[2]=JS_SLOPE*(abs(axis[2]))+JS_SENSITIVITY_YAW;
				js_sensitivity[3]=JS_SLOPE_LIFT*(JS_SHIFT-axis[3])+JS_SENSITIVITY_LIFT;

								
				js_RPYL[0] = axis[0] / js_sensitivity[0];
				js_RPYL[1] = axis[1] / js_sensitivity[1];
				js_RPYL[2] = axis[2] / js_sensitivity[2];
				js_RPYL[3] = -axis[3]/ js_sensitivity[3];
				
				//press fire button
				if (button[0]==1)				
					send_fire_button=1;					
			}
*/

			
		
		RPYL_value[0] = js_RPYL[0] + kb_RPYL[0] + RPYL_SHIFT;
		RPYL_value[1] = js_RPYL[1] + kb_RPYL[1] + RPYL_SHIFT;
		RPYL_value[2] = js_RPYL[2] + kb_RPYL[2] + RPYL_SHIFT;
		RPYL_value[3] = js_RPYL[3] + kb_RPYL[3] + RPYL_SHIFT;

		
		if (RPYL_value[0] <= 0) RPYL_value[0] = 0;
		if (RPYL_value[0] >= KB_MAX) RPYL_value[0] = KB_MAX;
		if (RPYL_value[1] <= 0) RPYL_value[1] = 0;
		if (RPYL_value[1] >= KB_MAX) RPYL_value[1] = KB_MAX;
		if (RPYL_value[2] <= 0) RPYL_value[2] = 0;
		if (RPYL_value[2] >= KB_MAX) RPYL_value[2] = KB_MAX;
		if (RPYL_value[3] <= 0) RPYL_value[3] = 0;
		if (RPYL_value[3] >= KB_MAX) RPYL_value[3] = KB_MAX;
		
		for (i=0;i<4;i++)
			RPYL_data[i] = (unsigned char) RPYL_value[i];

		clip_RPYL(RPYL_value,5);

		
		//printf("roll: %d pitch: %d yaw: %d lift: %d\n", RPYL_data[0],RPYL_data[1],RPYL_data[2],RPYL_data[3]);

		//printf("roll: %d pitch: %d yaw: %d lift: %d\n", kb_RPYL[0],kb_RPYL[1],js_RPYL[3],kb_RPYL[3]);
			
		oldroll = RPYL_data[0];
		oldpitch = RPYL_data[1];
		oldyaw = RPYL_data[2];
		oldlift = RPYL_data[3];


		if(1 == recv_data(&type, &data, &len)) 
		{		
			if (type==KEYESC)
			{
				quit=1;
			} else if (type == FEEDBACK_DATA) {
				//Interpret feedback data
				//Retrieve type
				feedbacktype = (data[5]-1)/2;
				data[5] = 0;
				//Make integer out of data structure
				feedbackvalues[feedbacktype] = make_int_swap(data);
				
				//Update 'GUI'
				system("clear");
				//printf("roll: %d pitch: %d yaw: %d lift: %d\n\n", RPYL_data[0],RPYL_data[1],RPYL_data[2],RPYL_data[3]);
				printf("mode:  %1d\n\n", feedbackvalues[0]);
				printf("roll:  %4d\tpitch: %4d\tyaw:   %4d\tlift:  %4d\n\n",feedbackvalues[1], feedbackvalues[2], feedbackvalues[3], feedbackvalues[4]);
				printf("oo1:   %4d\too2:   %4d\too3:   %4d\too4:   %4d\n\n",feedbackvalues[5], feedbackvalues[6], feedbackvalues[7], feedbackvalues[8]);
				printf("ax:    %4d\tay:    %4d\taz:    %4d\n\n", feedbackvalues[10], feedbackvalues[9], feedbackvalues[11]);
				printf("gyrox: %4d\tgyroy: %4d\tgyroz: %4d\n\n", feedbackvalues[12], feedbackvalues[13], feedbackvalues[14]);
				printf("p_yaw: %4d\tp1_full: %4d\tp2_full: %4d\n\n", feedbackvalues[15], feedbackvalues[16], feedbackvalues[17]);
				printf("control latency: %5d\n", feedbackvalues[18]);
			}	
		
			//Free buffer memory again (IMPORTANT!)
			free(data);
		}

	}
	
	/* stop timer */
  	ualarm(0, 0);

	printf("Exiting...\n");

	term_exitio();
//	joy_close();
	//Uninitialise
	comm_uninit();
	return 0;
}
