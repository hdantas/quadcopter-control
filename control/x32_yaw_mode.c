#include <stdio.h>
#include <stdlib.h>

#include "x32_common.h"
#include "comm.h"
#include "x32_modes.h"

void yaw_mode_ctrl()
{

		yaw_error=p_yaw*(yaw-s5);
		
		oo1 = (lift + 2 * pitch - yaw_error) / 4;
		oo2 = (lift - 2 * roll + yaw_error) / 4;
		oo3 = (lift - 2 * pitch - yaw_error) / 4;
		oo4 = (lift + 2 * roll + yaw_error) / 4;

/*		if ((oo1old!=oo1)||(oo2old!=oo2)||(oo3old!=oo3)||(oo4old!=oo4))*/
/*			printf("oo1: %d oo2: %d oo3: %d oo4: %d\n", oo1, oo2, oo3, oo4);*/
		
		return;		
}
