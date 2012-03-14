#include <stdio.h>
#include <stdlib.h>

#include "x32_common.h"
#include "comm.h"
#include "x32_modes.h"

#define SCALE_KALMAN 4100
#define P2PHI_INT 4300
#define INV_C1 4096
#define INV_C2_P2PHI 1023
#define FIXED_CONV1 1048576
#define FIXED_CONV2 1048576
#define FIXED_CONV3 4194304

int p_bias_roll, p_kalman_roll,phi_kalman_roll,phi_error_roll;
init =0

p_bias_roll=0 
phi_kalman_roll=0 
when I enter in full mode

//p s3 gyro x
//phi s0 accelerometer y

void kalman_filter_roll(void)
{
	p_kalman_roll= SCALE_KALMAN*s3 - p_bias_roll;
	phi_kalman_roll= phi_kalman_roll + (p_kalman_roll*P2PHI_INT)/FIXED_CONV1;
	phi_error_roll= phi_kalman_roll - SCALE_KALMAN*s0;
	phi_kalman_roll= phi_kalman_roll - (phi_error_roll*INV_C1)/FIXED_CONV2;
	p_bias_roll= p_bias_roll + (phi_error_roll*INV_C2_P2PHI)/FIXED_CONV3;
}
