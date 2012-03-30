#include <stdio.h>
#include <stdlib.h>

#include "x32_common.h"
#include "comm.h"
#include "x32_modes.h"
#include "filter.h"


int q_bias_pitch, q_kalman_pitch, theta_kalman_pitch, theta_error_pitch;
int p_bias_roll, p_kalman_roll,phi_kalman_roll,phi_error_roll;
int p2phi;

void reset_kalman(void)
{
	q_bias_pitch = 0;
	theta_kalman_pitch = 0;
	p_bias_roll = 0;
	phi_kalman_roll = 0;
}

void kalman_filter_pitch(void)
{
	q_kalman_pitch = (s4 << SCALE_SHIFT_KALMAN) - q_bias_pitch;
	theta_kalman_pitch = theta_kalman_pitch + ((q_kalman_pitch* p2phi) >> FIXED_CONV1);
	theta_error_pitch = theta_kalman_pitch - (s0 << SCALE_SHIFT_KALMAN);
	theta_kalman_pitch = theta_kalman_pitch - ((theta_error_pitch*INV_C1) >> FIXED_CONV2);
	q_bias_pitch = q_bias_pitch + ((theta_error_pitch* INV_C1_P2PHI) >> FIXED_CONV3);
}

void kalman_filter_roll(void)
{
	p_kalman_roll = (s3 << SCALE_SHIFT_KALMAN) - p_bias_roll;
	phi_kalman_roll = phi_kalman_roll + ((p_kalman_roll*p2phi) >> FIXED_CONV1);
	phi_error_roll = phi_kalman_roll - (s1 << SCALE_SHIFT_KALMAN);
	phi_kalman_roll = phi_kalman_roll - ((phi_error_roll*INV_C1) >> FIXED_CONV2);
	p_bias_roll = p_bias_roll + ((phi_error_roll*INV_C1_P2PHI) >> FIXED_CONV3);
}

void init_kalman(void)
{
	q_bias_pitch =  0;
	q_kalman_pitch = 0;
	theta_kalman_pitch = 0;
	theta_error_pitch = 0;
	p_bias_roll = 0;
	p_kalman_roll = 0;
	phi_kalman_roll = 0;
	phi_error_roll = 0;
	p2phi = INITIAL_P2PHI;
}
