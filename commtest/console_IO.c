/*------------------------------------------------------------
 * console I/O
 *------------------------------------------------------------
 */

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>


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
