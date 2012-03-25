#include <stdio.h>
#include <stdlib.h>

#include "x32_common.h"
#include "comm.h"
#include "x32_modes.h"

/*
In manual mode (mode 2) the ES simply passes on lift, roll, pitch, and yaw
*/

void manual_mode_ctrl(void)
{
	oo1 = SCALE_MANUAL * (lift + 2 * pitch - yaw) >> 2;
	oo2 = SCALE_MANUAL * (lift - 2 * roll + yaw) >> 2;
	oo3 = SCALE_MANUAL * (lift - 2 * pitch - yaw) >> 2;
	oo4 = SCALE_MANUAL * (lift + 2 * roll + yaw) >> 2;
}

