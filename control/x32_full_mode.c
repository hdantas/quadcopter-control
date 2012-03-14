#include <stdio.h>
#include <stdlib.h>

#include "x32_common.h"
#include "comm.h"
#include "x32_modes.h"
#include "kalman.h"
	
#define CAPACITY 256
#define CAPACITY_SHIFT 8 //256 = 2^8	

int latency[CAPACITY];
int i = 0;
int j = 0;
int starttime = 0;
int endtime = 0;
int average = 0;
	
void full_mode_ctrl(void)
{
	//roll = p1_full * (roll - s0) - p2_full * s3;
	//pitch = p1_full * (pitch - s1) - p2_full * s4;	

	starttime = X32_us_clock;
	
	yaw_mode_ctrl();
	
	kalman_filter_roll();
	roll = p1_full * (roll - phi_kalman_roll) - p2_full * p_kalman_roll;

	kalman_filter_pitch();
	pitch = p1_full * (pitch - theta_kalman_pitch) - p2_full * q_kalman_pitch;	
	
	endtime=X32_us_clock;
	
	latency[i++] = endtime-starttime;

	if (i == CAPACITY - 1)
	{	
		average = 0;
		for (j=0; j<CAPACITY; j++)
			average += latency[j];
		average = average << CAPACITY_SHIFT;
		printf("Average control latency : %d\n",average);
		i=0;
	}	

}
