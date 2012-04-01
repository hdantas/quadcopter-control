#include <stdio.h>
#include <stdlib.h>

#include "x32_common.h"
#include "comm.h"
#include "x32_modes.h"
#include "filter.h"

int s5_filtered, s5_old, s5_filtered_old;

/*Compute butterworth filter
* Author: Enrico Caruso */
void butterworth_filter (void) {
    s5_filtered=((B0*s5) >> FIXED_CONV4) + ((B1*s5_old) >> FIXED_CONV5) + ((A1*s5_filtered_old) >> FIXED_CONV6);
    s5_old=s5;
    s5_filtered_old=s5_filtered;
}

/*Initialize butterworth
* Author: Enrico Caruso */
void init_butterworth(void)
{
	s5_old=s5_filtered_old=0;
}
