#ifndef _SERVER_H
#define	_SERVER_H

int init();
void uninit();

int makenonblocking(int s);

void main(void);

void printbin(unsigned char c);

#endif
