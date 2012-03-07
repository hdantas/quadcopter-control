#include <stdio.h>
#include <stdlib.h>

#include "x32_common.h"
#include "comm.h"
#include "x32_modes.h"

void safe_mode_ctrl(void)
{
// safe mode (mode 0) the ES sends commands to the QR to shutdown the engines.

	oo1 = oo2 = oo3 = oo4 = 0;
}