#ifndef _SERIAL_H
#define _SERIAL_H

//Use serial.c for the PC version and x32_serial.c for the QR version

int serial_init();
void serial_uninit();

int serial_read(unsigned char* buffer);
int serial_write(unsigned char c);

#endif
