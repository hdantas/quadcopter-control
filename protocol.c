#define	FALSE		0
#define	TRUE		1

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>


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
		printf("\nread output %X",numberbytesread);
        	printf("\ngetChar = 0x ");
        	for(i=0;i<numberbytesread;i++)
	        	printf("%X ",*(line+i));
        	printf("\n");
        	
/*        	fseek (FILE *stream, long int offset, int whence)*/
/*        	The fseek function is used to change the file position of the stream stream. The value of whence must be one of the constants SEEK_SET, SEEK_CUR, or SEEK_END, to indicate whether the offset is relative to the beginning of the file, the current file position, or the end of the file, respectively.*/

/*This function returns a value of zero if the operation was successful, and a nonzero value to indicate failure. A successful call also clears the end-of-file indicator of stream and discards any characters that were “pushed back” by the use of ungetc.*/
		printf("value returned %X\n",line[numberbytesread-1]);
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

#define SERIAL_DEVICE 	"/dev/ttyUSB0"


int fd_RS232;


int rs232_open(void)
{
  	char 		*name;
  	int 		result;  
  	struct termios	tty;
  
        fd_RS232 = open(SERIAL_DEVICE, O_RDWR | O_NOCTTY);
        assert(fd_RS232>=0);

  	result = isatty(fd_RS232);
  	assert(result == 1);

  	name = ttyname(fd_RS232);
  	assert(name != 0);

  	result = tcgetattr(fd_RS232, &tty);
  	assert(result == 0);

        tty.c_iflag = IGNBRK; /* ignore break condition */
        tty.c_oflag = 0;
        tty.c_lflag = 0;

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; /* 8 bits-per-character */
        tty.c_cflag |= CLOCAL | CREAD; /* Ignore model status + read input */

        cfsetospeed(&tty, B115200); /* set output baud rate */
        cfsetispeed(&tty, B115200); /* set input baud rate */

        tty.c_cc[VMIN]  = 0;
        tty.c_cc[VTIME] = 0;

        tty.c_iflag &= ~(IXON|IXOFF|IXANY);

        result = tcsetattr (fd_RS232, TCSANOW, &tty); /* non-canonical */

        tcflush(fd_RS232, TCIOFLUSH); /* flush I/O buffer */
}


int 	rs232_close(void)
{
  	int 	result;

  	result = close(fd_RS232);
  	assert (result==0);
}


int	rs232_getchar_nb()
{
	int 		result;
	unsigned char 	c;

	result = read(fd_RS232, &c, 1);
	if (result == 0) {
		return -1;
	}
	else {
		assert(result == 1);   
		return (int) c;
	}
}


int 	rs232_getchar()
{
	int 	c;

	while ((c = rs232_getchar_nb()) == -1) 
		;
	return c;
}


int 	rs232_putchar(char c)
{ 
	int result;

	do {
		result = (int) write(fd_RS232, &c, 1);
	} while (result == 0);   

	assert(result == 1);
	return result;
}

int parity(char ino)
{
	int noofones = 0;
	char mask = 0x01; /* start at first bit */

	while(mask != 0) /* until all bits tested */
	{
		if(mask & ino) /* if bit is 1, increment noofones */
		{
			noofones++;
		}
		mask = mask << 1; /* go to next bit */
	}

	/* if noofones is odd, least significant bit will be 1 */

	return (noofones & 1); 
}

unsigned char source_button(char c) 
{	
	unsigned char source;
	switch (c) 
	{
		case 0x1B:	//ESC
			source=0;
			break;
		case 0x0A:	
			source=1;
			break;
		case '0':
			source=2;
			break;
		case '1':
			source=3;
			break;
		case '2':
			source=4;
			break;
		case '3':
			source=5;
			break;
		case '4':
			source=6;
			break;
		case '5':
			source=7;
			break;
		case 'a':
			source=8;
			break;
		case 'z':
			source=9;
			break;
		case 0x43://RIGHT
			source=10;
			break;
		case 0x44: //LEFT
			source=11;
			break;
		case 0x41://UP
			source=12;
			break;
		case 0x42: //DOWN
			source=13;
			break;
		case 'q':
			source=14;
			break;
		case 'w':
			source=15;
			break;
		case 'u':
			source=16;
			break;
		case 'j':
			source=17;
			break;
		case 'i':
			source=18;
			break;
		case 'k':
			source=19;
			break;
		case 'o':
			source=20;
			break;
		case 'l':
			source=21;
			break;
		default:
			printf("key not mapped error\n");
			return 'x';	//key not map
	}
	
	return  source;
	
	
}

/*Byte codification*/
/*A0 -> parity*/
/*A1 -> double/single*/
/*A2-A7 ->source*/

unsigned char protocol_button(char c)
{
	unsigned int byte; 		//packet
	unsigned int source;		//source
	
	source=source_button(c);
	if (source == 'x')
		return 'x';
	printf("source %d\t", source);

	
	if(parity(byte))	//if odd add a 1
		byte=(source & 0x3F)|(0x80);  //0x3F=00111111 0x80=10000000
	else
		byte=(source & 0x3F);

	printf("byte 0X%X\n", byte);
	
	return 	byte;

}

/*----------------------------------------------------------------
 * main -- execute terminal
 *----------------------------------------------------------------
 */
int main(void)
{
	
	char	c;

	unsigned char 	line[2];
	
	term_initio();
	rs232_open();

	/* discard any incoming text */
	 
	while ((c = rs232_getchar_nb()) != -1)
		fputc(c,stderr);
	
	//send "s" caracter to start the program in NEXIS
	strcpy(line,"s\n");
	rs232_putchar(line[0]);
	rs232_putchar(line[1]);

	for (;;) {
		if ((c = term_getchar_nb()) != -1) {
			//press a key button
			printf("\n\n****PC****\n");
				line[0]=protocol_button(c);
			if(line[0] != 'x')
				rs232_putchar(line[0]);
		}
		if ((c = rs232_getchar_nb()) != -1) {
			term_putchar(c);
		}
	}
	term_exitio();
	rs232_close();
	term_puts("\n<exit>\n");
  	return 0;
}



