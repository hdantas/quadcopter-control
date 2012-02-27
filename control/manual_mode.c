/*js_roll = axis[0] / JS_SENSITIVITY;
js_pitch = axis[1] / JS_SENSITIVITY;
js_yaw += (axis[2] / JS_SENSITIVITY) * DT;
js_lift = -(axis[3] - 32767) / JS_SENSITIVITY;*/

/*typedef struct {

  	double 	lift;	
  	double 	yaw;	
	double 	pitch;
	double 	roll;

} userstate_t;*/
/*--------------------------------------------------------------------------
 * initialize userstate signals (input to controller)
 *--------------------------------------------------------------------------
 */
/*void	user_init(userstate_t *userstate)
{
	kb_lift = kb_yaw = kb_pitch = kb_roll = 0;
	js_lift = js_yaw = js_pitch = js_roll = 0;
	userstate->lift = 0;
	userstate->yaw = 0;
	userstate->pitch = 0;
	userstate->roll = 0;
}*/

typedef enum {
		SAFE,
		PANIC,
		MANUAL,
		CALIBRATION,
		YAW,
		FULL
} ctr_mode;

ctr_mode mode;
unsigned char kb_lift, kb_yaw, kb_pitch, kb_roll;
unsigned char js_lift, js_yaw, js_pitch, js_roll;
double 	lift, roll, pitch, yaw;
double 	a1, a2, a3, a4;
double 	oo1, oo2, oo3, oo4;



/*--------------------------------------------------------------------------
 * keys
 *--------------------------------------------------------------------------
 */
void 	keyb(unsigned char key)
{
  	switch (key) {

		//TODO values of case are not right
			
		case 0x1B: /* ESC: abort / exit */
			break;
		case 0x0A: /*increment control mode */
			break;

		/* choose mode*/

		case '0': /*Safe Mode*/
			mode=0;
			break;
		case '1': /*Panic Mode*/
			mode=1;				
			break;
		case '2': /*Manual Mode*/
			mode=2;
			break;
		case '3': /*Calibration Mode*/
			mode=3;
			break;
		case '4': /*Yaw control Mode*/
			mode=4;
			break;
		case '5': /*Full control mode*/
			mode=5;
			break;

		/* quad rotor control*/	

		case 'a': /* increase lift */
			kb_lift += 0.5;
      			break;
		case 'z': /* decrease lift */
			kb_lift -= 0.5;
      			break;
		case 0x43: /*right arrow: roll down maybe*/
			kb_roll += 0.1;
			break;
		case 0x44: /*left arrow: roll up maybe*/
			kb_roll -= 0.1;
			break;
		case 0x41: /*up arrow: pitch down */
			kb_pitch -= 0.1;
			break;
		case 0x42: /*down arrow: pitch up */
			kb_pitch += 0.1;
			break;
		case 'w': /* increase yaw */
			kb_yaw += 0.2; 
      			break;
		case 'q': /* decrease yaw */
			kb_yaw -= 0.2; 
      			break;
		case 'u': /*yaw control P up*/
			break;
		case 'j': /*yaw control P down*/
			break;
		case 'i': /*roll/pitch control P1 up*/
			break;
		case 'k': /*roll/pitch control P1 down*/
			break;
		case 'o': /*roll/pitch control P2 up*/
			break;
		case 'l': /*roll/pitch control P2 down*/
			break;
		default:
			assert(0);
			break;
    	}
}

void set_actuators(mode)
{
	roll = kb_roll + js_roll;
	pitch = kb_pitch + js_pitch;
	yaw = kb_yaw + js_yaw;
	lift = kb_lift + js_lift;
	if (lift < 0) lift = 0; /*other controls on variables*/

	/* map lift, roll, pitch, yaw to rotor actuator vars ai
	 * so we need to solve for ai:
	 *
	 * b * (o1*o1 + o2*o2 + o3*o3 + o4*o4) = lift;
	 * b * (- o2*o2 + o4*o4) = roll;
	 * b * (o1*o1 - o3*o3) = pitch;
	 * d * (- o1*o1 + o2*o2 - o3*o3 + o4*o4) = yaw;
	 *
	 * let ooi be oi*oi. then we must solve
	 *
	 * [  1  1  1  1  ] [ oo1 ]   [lift/b]
	 * [  0 -1  0  1  ] [ oo2 ]   [roll/b]
	 *                          = 
	 * [ -1  0  1  0  ] [ oo3 ]   [pitch/b]
	 * [ -1  1 -1  1  ] [ oo4 ]   [yaw/d]
	 *
	 * the inverse matrix is
	 *
	 * [  1  0  2 -1  ]
	 * [  1 -2  0  1  ]
	 *                  * 1/4
	 * [  1  0 -2 -1  ]
	 * [  1  2  0  1  ]
	 *
	 * so with b = d = 1 we have
	 */
	oo1 = (lift + 2 * pitch - yaw) / 4;
	oo2 = (lift - 2 * roll + yaw) / 4;
	oo3 = (lift - 2 * pitch - yaw) / 4;
	oo4 = (lift + 2 * roll + yaw) / 4;
}





