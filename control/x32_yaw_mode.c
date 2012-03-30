#include <stdio.h>
#include <stdlib.h>

#include "x32_common.h"
#include "comm.h"
#include "x32_modes.h"
#include "filter.h"


void yaw_mode_ctrl()
{
	//yaw control
	ENABLE_INTERRUPT(INTERRUPT_OVERFLOW);

	butterworth_filter();
	yaw_error = p_yaw * ((yaw*SCALE_YAW) -  s5_filtered);

	//we set roll and pitch to zero for a better control of joystick
 	roll=0;
	pitch=0;

	oo1 = ((SCALE_LIFT*lift) - (yaw_error/SCALE_YAW_ERROR)) >> 2;
	oo2 = ((SCALE_LIFT*lift) + (yaw_error/SCALE_YAW_ERROR)) >> 2;
	oo3 = ((SCALE_LIFT*lift) - (yaw_error/SCALE_YAW_ERROR)) >> 2;
	oo4 = ((SCALE_LIFT*lift) + (yaw_error/SCALE_YAW_ERROR)) >> 2;

	DISABLE_INTERRUPT(INTERRUPT_OVERFLOW);

}
