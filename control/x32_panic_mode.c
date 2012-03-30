#include <stdio.h>
#include <stdlib.h>

#include "x32_common.h"
#include "comm.h"
#include "x32_modes.h"


void panic_mode_ctrl(void)
{

	int avarage;

	if (X32_ms_clock % 50 == 0){
		// Reduce by REDUNCING_RPM* motor* RPM until a predefined minumum
		
		avarage=(oo1+oo2+oo3+oo4) >> 2;

		oo1=avarage;
		oo2=avarage;
		oo3=avarage;
		oo4=avarage;
		
		if (oo1 > MIN_OO1)
			oo1 -= REDUCING_OO1;
		if (oo2 > MIN_OO2)
			oo2 -= REDUCING_OO2;
		if (oo3 > MIN_OO3)
			oo3 -= REDUCING_OO3;
		if (oo4 > MIN_OO4)
			oo4 -= REDUCING_OO4;
	}

	
	//if all motor are below a certain RPM move to safe mode
	if ((oo1 <= MIN_OO1) && (oo2 <= MIN_OO2) && (oo3 <= MIN_OO3) && (oo4 <= MIN_OO4))
		mode = SAFE;

	
}
