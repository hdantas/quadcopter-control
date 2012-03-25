#include <stdio.h>
#include <stdlib.h>

#include "x32_common.h"
#include "comm.h"
#include "x32_modes.h"

void safe_mode_ctrl(void)
{
// safe mode (mode 0) the ES sends commands to the QR to shutdown the engines.
	oo1=0;
	oo2=0;
	oo3=0;
	oo4=0;
	
	if (terminate)
		finished = 1; //if program was asked to terminate set finished flag to return control to PC
	
}
