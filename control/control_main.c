#include "console_IO.h"
#include "comm.h"
#include "joystick.h"
#include <stdio.h>
#include <stdlib.h>
#define JS_SENSITIVITY 512

extern int *axis;		//vector for axes values
extern char *button;		//vector for button value
int kb_RPYL[4];
int js_RPYL[4];

//gcc -o main main.c comm.c checksum.c serial.c convert.c joy_function.c console_IO.c

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
			if(kb_RPYL[3]<127)
				kb_RPYL[3]+= 1;
			return 0;
		case 'z':		/* decrease lift */
			*type_c=KEYZ;
			if(kb_RPYL[3]>0)
				kb_RPYL[3]-=1;
			return 0;
		case 0x43:		/*right arrow: roll down maybe*/
			*type_c=KEYRIGHT;
			if(kb_RPYL[0]<127)
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
			if(kb_RPYL[1]<127)
				kb_RPYL[1] += 1;
			return 0;
		case 'q':		/* decrease yaw */
			*type_c=KEYQ;
			if(kb_RPYL[2]>0)
				kb_RPYL[2] -= 1;
			return 0;
		case 'w':		/* increase yaw */
			*type_c=KEYW;
			if(kb_RPYL[2]<127)
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
			printf("key not mapped error\n");
			return -1;	//key not map
	}
}

int main(void) {
	//Declare variables
	int RPYL_value[4];
	comm_type type;
	unsigned char* data;
	unsigned char RPYL_data[4];
	int len;
	int i;
	int exit;
	char c;
	comm_type type_c;

	exit=0;

	kb_RPYL[0]=kb_RPYL[1]=kb_RPYL[2]=64;
	kb_RPYL[3]=0;
	js_RPYL[0]=0;//TODO
	
	//Initialise communication
	if (0 != comm_init())
		return -1;

	term_initio();

/*	joy_open();*/
	

	while(!exit) {

		if ((c = term_getchar_nb()) != -1) 
		{
			//press a key button	
			if (source_button(c, &type_c)==1)
			{
				if (0 != send_data(type_c, 0, 0))
				{
					printf("Error sending buttons\n");
					break;
				}
			}

		}
		
	
/*		if(read_joy()!= 0) */
/*			{*/
/*				js_RPYL[0]=(axis[0]+32767) / JS_SENSITIVITY;*/
/*				js_RPYL[1]=(axis[1]+32767) / JS_SENSITIVITY;*/
/*				js_RPYL[2] += ((axis[2]+32767) / JS_SENSITIVITY);*/
/*				js_RPYL[3] = -(axis[3] - 32767) / JS_SENSITIVITY;*/

/*				//press fire button*/
/*				if (button[15]==1)*/
/*					if (0 != send_data(KEYESC, 0, 0))*/
/*					{*/
/*						printf("Error sending buttons\n");*/
/*						break;*/
/*					}*/
/*			}*/


			
		
		RPYL_value[0]=js_RPYL[0]+kb_RPYL[0];
		RPYL_value[1]=js_RPYL[1]+kb_RPYL[1];
		RPYL_value[2]=js_RPYL[2]+kb_RPYL[2];
		RPYL_value[3]=js_RPYL[3]+kb_RPYL[3];
		
		if (RPYL_value[0] <= 0) RPYL_value[0] = 0;
		if (RPYL_value[0] >= 127) RPYL_value[0] = 127;
		if (RPYL_value[1] <= 0) RPYL_value[1] = 0;
		if (RPYL_value[1] >= 127) RPYL_value[1] = 127;
		if (RPYL_value[2] <= 0) RPYL_value[2] = 0;
		if (RPYL_value[2] >= 127) RPYL_value[2] = 127;
		if (RPYL_value[3] <= 0) RPYL_value[3] = 0;
		if (RPYL_value[3] >= 127) RPYL_value[3] = 127;

		for (i=0;i<4;i++)
			RPYL_data[i]=(unsigned char) RPYL_value[i];
		
		printf("roll: %d pitch: %d yaw: %d lift: %d\n", RPYL_data[0],RPYL_data[1],RPYL_data[2],RPYL_data[3]);
	
			
		if (0 != send_data(RPYL, RPYL_data, 4))
		{
			printf("Error sending joystick\n");
			break;		
		}

		usleep(5000);

		if(1 == recv_data(&type, &data, &len)) 
		{		
			if (type==KEYESC)
			{
				exit=1;
			}	
		
			//Free buffer memory again (IMPORTANT!)
			free(data);
		}
		
	}

	printf("Exiting...\n");

	term_exitio();
/*	joy_close();*/
	//Uninitialise
	comm_uninit();
	return 0;
}
