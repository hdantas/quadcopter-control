#include "serial.h"
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

//On linux, use serial connection /dev/ttyS0, or /dev/ttyUSB0 for USB-to-serial cable
#define SERIAL_DEVICE	"/dev/ttyUSB0"

//Global variables
int serial_handle;

/*	Returns 0 if successful, -1 otherwise
*/
int serial_init() {
	//Declare variables
	struct termios tty;

	//Open RS232 link
	serial_handle = open(SERIAL_DEVICE, O_RDWR | O_NOCTTY);	//Read-write, not controlling terminal	
	//Verify handle
	if (isatty(serial_handle) != 1)
		return -1;

	//Get attributes
	if (tcgetattr(serial_handle, &tty) != 0)
		return -1;

	//Flags
	tty.c_iflag = IGNBRK & ~(IXON | IXOFF | IXANY);
	tty.c_oflag = 0;
	tty.c_lflag = 0;
	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8	//8 bit character size
				|	CLOCAL | CREAD;	//Enable receive, ignore modem status lines
	
	//Baud rate
	cfsetospeed(&tty, B115200);
	cfsetispeed(&tty, B115200);

	tty.c_cc[VMIN] = 0;
	tty.c_cc[VTIME] = 0;
	
	//Apply
	if (tcsetattr(serial_handle, TCSANOW, &tty) != 0)
		return -1;
	if (tcflush(serial_handle, TCIOFLUSH) != 0)
		return -1;

	return 0;
}

void serial_uninit() {
	//Close RS232 link
	close(serial_handle);
	serial_handle = 0;
}

/*	Returns 1 if byte was read and placed in buffer, 0 otherwise
*/
int serial_read(unsigned char* buffer) {
	//Attempt to read single byte from serial link
	return read(serial_handle, buffer, 1);
}

/*	Returns 1 if byte was written successfully, 0 if busy, -1 on error.
*/
int serial_write(unsigned char c) {
	//Declare Variables
	int result;

	//Execute blocking write
	do {
		result = write(serial_handle, &c, 1);
	} while (result == 0);

	return result;
}

