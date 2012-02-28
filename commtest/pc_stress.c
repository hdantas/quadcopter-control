/*I send RPYL (always the same data)*/
/*and I print the number of packets that I recive in NEXYS and the time necessary to process the packet.*/
/*I measure the time beetwen the interruption call and when the data of packet are ready to use  */

/*I use printf in NEXIS side and so for see this information you must open a terminal and run myterm program*/

/*the corisponding c file in NEXYS side it is x32_stress.c*/

/*gcc -o pc_stress pc_stress.c comm.c checksum.c serial.c convert.c*/

/*lcc-x32 x32_stress.c comm.c checksum.c x32_serial_time.c convert.c -o x32_stress.ce*/

#define	FALSE		0
#define	TRUE		1
#define JS_SENSITIVITY 512
#define iteration 500

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include "comm.h"


/*------------------------------------------------------------
 * console I/O
 *------------------------------------------------------------
 */
struct termios 	savetty;


void	term_initio()
{
	struct termios tty;

        tcgetattr(0, &savetty);
        tcgetattr(0, &tty);
        tty.c_lflag &= ~(ECHO|ECHONL|ICANON|IEXTEN);
        tty.c_cc[VTIME] = 0;
        tty.c_cc[VMIN] = 0;
        tcsetattr(0, TCSADRAIN, &tty);
}

void	term_exitio()
{
	tcsetattr(0, TCSADRAIN, &savetty);
}

void	term_puts(char *s) 
{ 
	fprintf(stderr,"%s",s); 
}

void	term_putchar(char c) 
{ 
	putc(c,stderr); 
}

int	term_getchar_nb() 
{ 
	static unsigned char line [4];

	// note: destructive read
/*file descriptors 4 linux	*/
/*0 Standard Input (Generally Keyboard)*/
/*1 Standard output (Generally Display/Screen)*/
/*2 Standard Error Ouput (Generally Display/Screen)*/
	int numberbytesread = read(0,line,4);
	int i;
        if (numberbytesread) {
        	for(i=0;i<numberbytesread;i++)
	                return (int) line[numberbytesread-1];
        }
        return -1;
}

int	term_getchar() 
{ 
        int    c;

        while ((c = term_getchar_nb()) == -1)
                ;
        return c;
}

/*------------------------------------------------------------
 * serial I/O (8 bits, 1 stopbit, no parity, 115,200 baud)
 *------------------------------------------------------------
 */
#include <termios.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>


int main(void) {
	//Declare variables
	unsigned char RPYL_value[4];
	comm_type type;
	unsigned char* data;
	int len;
	int i;
	int send_cnt;
	int error_send, error_data, error_receive;
	char c;
	comm_type tempc;
	for (i=0;i<4;i++)
		RPYL_value[i]=0;	
	//Initialise communication
	if (0 != comm_init())
		return -1;

	term_initio();
	
	//Create dummy axes values
	RPYL_value[0] = 0x05;	RPYL_value[1] = 0x17;
	RPYL_value[2] = 0x42;	RPYL_value[3] = 0x00;
	
	error_send=error_data=error_receive=0;
	send_cnt=0;

	while (send_cnt<iteration) {
		
			//Send
			if (0 != send_data(RPYL, RPYL_value, 4))
				error_send++;
			else 
				send_cnt++;

		usleep(5000);

	}

	printf("send_cnt %d\nerror_send %d\n", send_cnt,error_send);
	send_data(KEYESC, 0, 0);
	term_exitio();
	//Uninitialise
	comm_uninit();
	return 0;
}
