#include <stdio.h>
#include <stdlib.h>

#include "x32_common.h"
#include "comm.h"
#include "x32_modes.h"


void panic_mode_ctrl(void)
{
	if (X32_ms_clock % 500 == 0){
		// Reduce by REDUNCING_RPM* motor* RPM until a predefined minumum
		if (roll > MIN_ROLL)
			roll -= REDUCING_ROLL;
		if (pitch > MIN_PITCH)
			pitch -= REDUCING_PITCH;
		if (yaw > MIN_YAW)
			yaw -= REDUCING_YAW;
		if (lift > MIN_LIFT)
			lift -= REDUCING_LIFT;
	}
	
	//if all motor are below a certain RPM move to safe mode
	if ((roll <= MIN_ROLL) && (pitch <= MIN_PITCH) && (yaw <= MIN_YAW) && (lift <= MIN_LIFT))
		mode = SAFE;

	
}
