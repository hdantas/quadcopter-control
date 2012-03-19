#include <stdio.h>
#include <stdlib.h>

#include "x32_common.h"
#include "comm.h"
#include "x32_modes.h"

void yaw_mode_ctrl()
{
	yaw = p_yaw * (yaw_controller - s5);
}
