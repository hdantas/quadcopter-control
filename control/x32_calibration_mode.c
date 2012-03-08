#include <stdio.h>
#include <stdlib.h>

#include "x32_common.h"
#include "comm.h"
#include "x32_modes.h"

void calibration_mode(void)
{
	s0_bias = s0;
	s1_bias = s1;
	s2_bias = s2;
	s3_bias = s3;
	s4_bias = s4;
	s5_bias = s5;
	return;
}
