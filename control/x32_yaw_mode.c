#include <stdio.h>
#include <stdlib.h>

#include "x32_common.h"
#include "comm.h"
#include "x32_modes.h"


void yaw_mode_ctrl()
{
	yaw_error = p_yaw * ((yaw*SCALE_YAW) - s5);
 	roll=0;
	pitch=0;

	oo1 = ((SCALE_LIFT*lift) - (SCALE_YAW_ERROR*yaw_error)) >> 2;
	oo2 = ((SCALE_LIFT*lift) + (SCALE_YAW_ERROR*yaw_error)) >> 2;
	oo3 = ((SCALE_LIFT*lift) - (SCALE_YAW_ERROR*yaw_error)) >> 2;
	oo4 = ((SCALE_LIFT*lift) + (SCALE_YAW_ERROR*yaw_error)) >> 2;

/*	if (X32_ms_clock % 100 == 0)*/
/*		printf("%d %d %d\n", yaw_error, yaw, s5);*/
}
