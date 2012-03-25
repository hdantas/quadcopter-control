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
	//int starttime, endtime;


	//starttime = X32_us_clock;
	
	ENABLE_INTERRUPT(INTERRUPT_OVERFLOW);

	
	yaw_error = p_yaw * ((yaw*SCALE_YAW) - s5);

	kalman_filter_roll();
	roll_error = p1_full * ((roll*SCALE_ROLL) - (phi_kalman_roll >> SHIFT_PHI_KALMAN_ROLL)) - p2_full * (p_kalman_roll >> SHIFT_P_KALMAN_ROLL);

	kalman_filter_pitch();
	pitch_error = p1_full * ((pitch*SCALE_PITCH) - (theta_kalman_pitch >> SHIFT_THETA_KALMAN_PITCH)) - p2_full * (q_kalman_pitch >> SHIFT_Q_KALMAN_PITCH);


	oo1 = ((SCALE_LIFT*lift) + 2 * (pitch_error / SCALE_PITCH_ERROR) - (SCALE_YAW_ERROR*yaw_error)) >> 2;
	oo2 = ((SCALE_LIFT*lift) - 2 * (roll_error / SCALE_ROLL_ERROR) + (SCALE_YAW_ERROR*yaw_error)) >> 2;
	oo3 = ((SCALE_LIFT*lift) - 2 * (pitch_error / SCALE_PITCH_ERROR) - (SCALE_YAW_ERROR*yaw_error)) >> 2;
	oo4 = ((SCALE_LIFT*lift) + 2 * (roll_error / SCALE_ROLL_ERROR) + (SCALE_YAW_ERROR*yaw_error)) >> 2;

	DISABLE_INTERRUPT(INTERRUPT_OVERFLOW);

	/*endtime=X32_us_clock;
	if (X32_ms_clock % 100 == 0)
		printf("%d\n", endtime-starttime);*/
}
		
