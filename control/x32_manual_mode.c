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
		
		if (roll > MAX_ROLL) roll = MAX_ROLL;
		if (roll < MIN_ROLL) roll = MIN_ROLL;
		if (pitch < MIN_PITCH) lift = MIN_PITCH;
		if (pitch > MAX_PITCH) lift = MAX_PITCH;		
		if (yaw > MAX_YAW) yaw = MAX_YAW;
		if (yaw < MIN_YAW) yaw = MIN_YAW;
		if (lift < MIN_LIFT) lift = MIN_LIFT;
		if (lift > MAX_LIFT) lift = MAX_LIFT;
		
		
		oo1 = (lift + 2 * pitch - yaw) / 4;
		oo2 = (lift - 2 * roll + yaw) / 4;
		oo3 = (lift - 2 * pitch - yaw) / 4;
		oo4 = (lift + 2 * roll + yaw) / 4;

/*		if ((oo1old!=oo1)||(oo2old!=oo2)||(oo3old!=oo3)||(oo4old!=oo4))*/
/*			printf("oo1: %d oo2: %d oo3: %d oo4: %d\n", oo1, oo2, oo3, oo4);*/
		
		return;
}

