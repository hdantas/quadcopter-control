#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include "joystick.h"

#define NAME_LENGTH 128

int fd_JOY;
unsigned char axes;	//number of axes	
unsigned char buttons;	//number of buttons
int *axis;		//vector for axes values
char *button;		//vector for button value


int joy_open(void)
{
	int version;		//version of joy
	char name[NAME_LENGTH];	//name of joy
	int i;

/*	struct js_event {
	__u32 time;	event timestamp in milliseconds
	__s16 value;	value 
	__u8 type;	event type
	__u8 number;	axis/button number
};*/
  	
	if ((fd_JOY = open("/dev/input/js0", O_RDONLY)) < 0) 
    	{
		perror("jstest");
		exit(1);
	}

	ioctl(fd_JOY, JSIOCGVERSION, &version);
	ioctl(fd_JOY, JSIOCGAXES, &axes);
	ioctl(fd_JOY, JSIOCGBUTTONS, &buttons);
	ioctl(fd_JOY, JSIOCGNAME(NAME_LENGTH), name);

	axis = calloc(axes, sizeof(int));
	button = calloc(buttons, sizeof(char));
    
    	fcntl(fd_JOY, F_SETFL, O_NONBLOCK); //non-blocking mode

  	return 1;
}

int 	joy_close(void)
{
  	int 	result;

  	result = close(fd_JOY);
  	assert (result==0);
}

int	read_joy()
{ 
	struct js_event js;
	int button_press;
	int axis_move;

	button_press=axis_move=0;
	

	while (read(fd_JOY, &js, sizeof(struct js_event)) == sizeof(struct js_event))  
        	{
			switch(js.type & ~JS_EVENT_INIT) 
			{
				case JS_EVENT_BUTTON:
					button[js.number] = js.value;
					button_press=1;
					break;
				case JS_EVENT_AXIS:
					axis[js.number] = js.value;
					axis[js.number]=axis[js.number]; //joy has +-32768 values,  +-32768/512=+-64 values and so 128 values (7 bits)
					axis_move=1;
					break;
			}
			
		}

	if (errno != EAGAIN) 
        	{
			perror("\njstest: error reading");
			exit (1);
		}

	if ((button_press==0)&&(axis_move==0))	//no read
		return 0;
	if ((button_press==0)&&(axis_move==1))	//read axis, but no button press
		return 1;	
	if ((button_press==1)&&(axis_move==0))	//read button, but no axis move
		return 2;
	if ((button_press==1)&&(axis_move==1))	//read buttons and axis
		return 3;
	
}
