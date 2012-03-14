#include <stdio.h>
#include <stdlib.h>

#include "x32_common.h"
#include "comm.h"
#include "x32_modes.h"

void full_mode_ctrl(void)
{
	yaw_mode_ctrl();
	roll = p1_full * (roll - s0) - p2_full * s3;
	pitch = p1_full * (pitch - s1) - p2_full * s4;	

}
