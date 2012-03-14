#include <stdio.h>
#include <stdlib.h>

#include "x32_common.h"
#include "comm.h"
#include "x32_modes.h"

//common with kalman roll not need

#define SCALE_KALMAN 4100
#define INV_C1 4096
#define FIXED_CONV1 1048576
#define FIXED_CONV2 1048576
#define FIXED_CONV3 16777216

int p_bias_pitch, p_kalman_pitch,phi_kalman_pitch,phi_error_pitch;
init all =0, but p2phi=4300

p_bias_pitch=0 
phi_kalman_pitch=0 
when I enter in full mode

//q gyro y s4
//theta s1 accelerometer x


void kalman_filter_pitch(void)
{
	p_kalman_pitch= SCALE_KALMAN*s4 - p_bias_pitch;
	phi_kalman_pitch= phi_kalman_pitch + (p_kalman_pitch* p2phi)/FIXED_CONV1;
	phi_error_pitch= phi_kalman_pitch - SCALE_KALMAN*s1;
	phi_kalman_pitch= phi_kalman_pitch - (phi_error_pitch*INV_C1)/FIXED_CONV2;
	p_bias_pitch= p_bias_pitch + (phi_error_pitch* p2phi)/FIXED_CONV3;
}
