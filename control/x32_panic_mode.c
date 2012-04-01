#include <stdio.h>
#include <stdlib.h>

#include "x32_common.h"
#include "comm.h"
#include "x32_modes.h"

/* Author: Henrique Dantas */
void panic_mode_ctrl(void)
{

	int average;

	//Reduce engien RPM envery 50ms, otherwise it goes down too quickly
	if (X32_ms_clock % 50 == 0){	
		//Average the motors to make descent vertical
		average=(oo1+oo2+oo3+oo4) >> 2;

		oo1=average;
		oo2=average;
		oo3=average;
		oo4=average;
		
		// Reduce by REDUNCING_RPM*motor* until a predefined minumum
		if (oo1 > MIN_OO1)
			oo1 -= REDUCING_OO1;
		if (oo2 > MIN_OO2)
			oo2 -= REDUCING_OO2;
		if (oo3 > MIN_OO3)
			oo3 -= REDUCING_OO3;
		if (oo4 > MIN_OO4)
			oo4 -= REDUCING_OO4;
	}

	
	//if all motor are below a certain RPM move to safe mode
	if ((oo1 <= MIN_OO1) && (oo2 <= MIN_OO2) && (oo3 <= MIN_OO3) && (oo4 <= MIN_OO4))
		mode = SAFE;

	
}
