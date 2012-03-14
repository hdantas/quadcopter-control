#include <stdio.h>
#include <stdlib.h>

#include "x32_common.h"
#include "comm.h"
#include "x32_modes.h"
#include "kalman.h"

void full_mode_ctrl(void)
{
	//roll = p1_full * (roll - s0) - p2_full * s3;
	//pitch = p1_full * (pitch - s1) - p2_full * s4;	

	yaw_mode_ctrl();
	
	kalman_filter_roll();
	roll = p1_full * (roll - phi_kalman_roll) - p2_full * p_kalman_roll;

	kalman_filter_pitch();
	pitch = p1_full * (pitch - theta_kalman_pitch) - p2_full * q_kalman_pitch;	

}
