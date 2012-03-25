#include <stdio.h>
#include <stdlib.h>

#include "x32_common.h"
#include "comm.h"
#include "x32_modes.h"

void calibration_mode(void)
{
	s0_bias = X32_QR_s0;
	s1_bias = X32_QR_s1;
	s2_bias = X32_QR_s2;
	s3_bias = X32_QR_s3;
	s4_bias = X32_QR_s4;
	s5_bias = X32_QR_s5;
	return;
}
