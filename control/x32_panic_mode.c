#include <stdio.h>
#include <stdlib.h>

#include "x32_common.h"
#include "comm.h"
#include "x32_modes.h"


void panic_mode_ctrl(void)
{

	int average;

	if (X32_ms_clock % 50 == 0){
		// Reduce by REDUNCING_RPM* motor* RPM until a predefined minumum
		// Stabilize QR by setting all motors with the same value
		average=(oo1+oo2+oo3+oo4)>>2;

		oo1=average;
		oo2=average;
		oo3=average;
		oo4=average;
		
		if (average > MIN_MOTORS) {
			oo1 -= REDUCING_RPM;
			oo2 -= REDUCING_RPM;
			oo3 -= REDUCING_RPM;
			oo4 -= REDUCING_RPM;
		}
	}

	
	//if all motor are below a certain RPM move to safe mode
	if ((oo1 <= MIN_MOTORS) && (oo2 <= MIN_MOTORS) && (oo3 <= MIN_MOTORS) && (oo4 <= MIN_MOTORS)){
		mode = SAFE;
		terminate = 1;
	}
	
}
