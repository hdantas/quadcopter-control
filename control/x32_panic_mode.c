#include <stdio.h>
#include <stdlib.h>

#include "x32_common.h"
#include "comm.h"
#include "x32_modes.h"

/*	In panic mode (mode 1) the ES commands the engines to moderate RPM for a few seconds such that the
*	QR (assumed uncontrollable) will still make a somewhat controlled landing to avoid structural damage.
*	In the panic state, the ES should ignore any command from the PC link, and after a few seconds should
*	autonomously enter safe mode.
*/

void panic_mode_ctrl(void)
{
	// Reduce by REDUNCING_RPM* motor* RPM until a predefined minumum
	if (oo1 > MIN_MOTOR1)
		oo1 -= REDUCING_RPM1;
	if (oo2 > MIN_MOTOR2)
		oo2 -= REDUCING_RPM2;
	if (oo3 > MIN_MOTOR3)
		oo3 -= REDUCING_RPM3;
	if (oo4 > MIN_MOTOR4)
		oo4 -= REDUCING_RPM4;
		
	//if all motor are below a certain RPM move to safe mode
	if ((oo1 <= MIN_MOTOR1) && (oo2 <= MIN_MOTOR2) && (oo3 <= MIN_MOTOR3) && (oo4 <= MIN_MOTOR4))
		mode = SAFE;

	
}
