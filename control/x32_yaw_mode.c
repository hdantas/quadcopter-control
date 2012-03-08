#include <stdio.h>
#include <stdlib.h>

#include "x32_common.h"
#include "comm.h"
#include "x32_modes.h"

void yaw_mode_ctrl()
{
	int yaw_error;

	yaw_error=p_yaw*(yaw-(s5-s5_bias));
	oo1 = (lift + 2 * pitch - yaw_error) / 4;
	oo2 = (lift - 2 * roll + yaw_error) / 4;
	oo3 = (lift - 2 * pitch - yaw_error) / 4;
	oo4 = (lift + 2 * roll + yaw_error) / 4;

	return;		
}
