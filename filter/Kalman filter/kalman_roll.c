#include <stdio.h>
#include <stdlib.h>

#include "x32_common.h"
#include "comm.h"
#include "x32_modes.h"

#define SCALE_KALMAN 4100
#define INV_C1 4096
#define FIXED_CONV1 1048576
#define FIXED_CONV2 1048576
#define FIXED_CONV3 16777216


int p_bias_roll, p_kalman_roll,phi_kalman_roll,phi_error_roll, p2phi;
init all =0, but p2phi=4300

p_bias_roll=0 
phi_kalman_roll=0 
when I enter in full mode

//p s3 gyro x
//phi s0 accelerometer y

void kalman_filter_roll(void)
{
	p_kalman_roll= SCALE_KALMAN*s3 - p_bias_roll;
	phi_kalman_roll= phi_kalman_roll + (p_kalman_roll*p2phi)/FIXED_CONV1;
	phi_error_roll= phi_kalman_roll - SCALE_KALMAN*s0;
	phi_kalman_roll= phi_kalman_roll - (phi_error_roll*INV_C1)/FIXED_CONV2;
	p_bias_roll= p_bias_roll + (phi_error_roll*p2phi)/FIXED_CONV3;
}
