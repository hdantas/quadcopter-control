#include <stdio.h>
#include <stdlib.h>

#include "x32_common.h"
#include "comm.h"
#include "x32_modes.h"

/* Author: Henrique Dantas */
void calibration_mode(void)
{
	//read and store sensore value
	s0_bias = X32_QR_s0;
	s1_bias = X32_QR_s1;
	s2_bias = X32_QR_s2;
	s3_bias = X32_QR_s3;
	s4_bias = X32_QR_s4;
	s5_bias = X32_QR_s5;
	
	//set engine values to zero
	oo1=0;
	oo2=0;
	oo3=0;
	oo4=0;
	return;
}
