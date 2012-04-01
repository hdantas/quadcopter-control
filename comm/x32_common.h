#ifndef _X32_COMMON_H
#define _X32_COMMON_H

/*	Author: Maurijn Neumann
 */

//Comment this out to compile for the QR, leave it in to compile for Nexys board
#define NEXYS

//Include proper header
#ifdef NEXYS
	#include "x32_nexys.h"
#else
	#include "x32.h"
#endif

//For malloc to work it requires a pre-allocated memory buffer
//Note that 1kB is gross overkill, as comm.c uses a few bytes at most
char malloc_memory[1024];
int malloc_memory_size = 1024;

//LED map
#define STATUS_LED	0x01
#define COMM_LED	0x02
#define QR_LINK_LED	0x04

#endif
