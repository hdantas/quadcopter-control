#include <stdio.h>
#include <stdlib.h>

#include "x32_common.h"
#include "comm.h"
#include "x32_modes.h"
#include "filter.h"
	
/* Author: Enrico Caruso */	
void full_mode_ctrl(void)
{
	//overflow detaction	
	ENABLE_INTERRUPT(INTERRUPT_OVERFLOW);

	//yaw control
	butterworth_filter();
	yaw_error = p_yaw * ((yaw*SCALE_YAW) -  s5_filtered);

	//stimation of phi and p
	kalman_filter_roll();
	//roll control
	roll_error = p1_full * ((roll*SCALE_ROLL) - (phi_kalman_roll >> SHIFT_PHI_KALMAN_ROLL)) - p2_full * (p_kalman_roll >> SHIFT_P_KALMAN_ROLL);

	//stimation of theta and q
	kalman_filter_pitch();
	//pitch control
	pitch_error = p1_full * ((pitch*SCALE_PITCH) - (theta_kalman_pitch >> SHIFT_THETA_KALMAN_PITCH)) - p2_full * (q_kalman_pitch >> SHIFT_Q_KALMAN_PITCH);


	oo1 = ((SCALE_LIFT*lift) + 2 * (pitch_error / SCALE_PITCH_ERROR) - (yaw_error/SCALE_YAW_ERROR)) >> 2;
	oo2 = ((SCALE_LIFT*lift) - 2 * (roll_error / SCALE_ROLL_ERROR) + (yaw_error/SCALE_YAW_ERROR)) >> 2;
	oo3 = ((SCALE_LIFT*lift) - 2 * (pitch_error / SCALE_PITCH_ERROR) - (yaw_error/SCALE_YAW_ERROR)) >> 2;
	oo4 = ((SCALE_LIFT*lift) + 2 * (roll_error / SCALE_ROLL_ERROR) + (yaw_error/SCALE_YAW_ERROR)) >> 2;

	DISABLE_INTERRUPT(INTERRUPT_OVERFLOW);

}
		
