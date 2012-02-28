/*I send RPYL (always the same data)*/
/*I count the number of packets send and receive*/
/*I check if the data received are equal to the one that I have sent*/
/*If I don't receive all the packets I can quit with q button and see how many packets I have lost*/

/*the corisponding c file in NEXYS side it is x32_loopback_stress.c*/

/*gcc -o pc_loopback_stress pc_loopback_stress.c comm.c checksum.c serial.c convert.c*/

/*lcc-x32 x32_loopback_stress.c comm.c checksum.c x32_serial.c convert.c -o x32_loopback_stress.ce*/


#define	FALSE		0
#define	TRUE		1
#define JS_SENSITIVITY 512
#define iteration 50

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
	int I;
	int send_cnt,receive_cnt;
	int error_send, error_data, error_receive;
	char c;
	comm_type tempc;
	for (I=0;I<4;I++)
		RPYL_value[I]=0;	
	//Initialise communication
	if (0 != comm_init())
		return -1;

	term_initio();
	printf("Press q to exit\n\n\n");
	
	//Create dummy axes values
	RPYL_value[0] = 0x05;	RPYL_value[1] = 0x17;
	RPYL_value[2] = 0x42;	RPYL_value[3] = 0x00;
	
	error_send=0;
	error_data=0;
	send_cnt=0;
	receive_cnt=0;
	error_receive=0;

	while ((send_cnt!=iteration)||(receive_cnt!=iteration)) {
		if ((c = term_getchar_nb()) != -1) 
		{
			if (c=='q')
				break;
		}
		
		if (send_cnt<iteration)
		{
			//Send
			if (0 != send_data(RPYL, RPYL_value, 4))
				error_send++;
			else 
				send_cnt++;
		}

		usleep(100000);

		if(1 == recv_data(&type, &data, &len)) 
		{		
			if (type==RPYL)
			{
				if ((data[0]!=0x05) || (data[1]!= 0x17) || (data[2]!=0x42) || (data[3]!= 0x00))
					error_data++;
			}	
			else
				error_receive++;

			receive_cnt++;
			//Free buffer memory again (IMPORTANT!)
			free(data);
		}
	}

	printf("send_cnt %d\nreceive_cnt %d\nerror_receive %d\nerror_send %d\nerror_data %d\n", send_cnt, receive_cnt, error_receive, error_send, error_data);
	term_exitio();
	//Uninitialise
	comm_uninit();
	return 0;
}
