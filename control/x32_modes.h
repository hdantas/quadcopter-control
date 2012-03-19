#ifndef _X32_MODES_H
#define _X32_MODES_H

#define X32_timer_per           peripherals[PERIPHERAL_TIMER1_PERIOD]
#define X32_leds		peripherals[PERIPHERAL_LEDS]
#define X32_ms_clock		peripherals[PERIPHERAL_MS_CLOCK]
#define X32_us_clock		peripherals[PERIPHERAL_US_CLOCK]
#define X32_QR_a0 		peripherals[PERIPHERAL_XUFO_A0]
#define X32_QR_a1 		peripherals[PERIPHERAL_XUFO_A1]
#define X32_QR_a2 		peripherals[PERIPHERAL_XUFO_A2]
#define X32_QR_a3 		peripherals[PERIPHERAL_XUFO_A3]
#define X32_QR_s0 		peripherals[PERIPHERAL_XUFO_S0] //sax
#define X32_QR_s1 		peripherals[PERIPHERAL_XUFO_S1] //say
#define X32_QR_s2 		peripherals[PERIPHERAL_XUFO_S2] //saz
#define X32_QR_s3 		peripherals[PERIPHERAL_XUFO_S3] //sp
#define X32_QR_s4 		peripherals[PERIPHERAL_XUFO_S4] //sq
#define X32_QR_s5 		peripherals[PERIPHERAL_XUFO_S5] //sr
#define X32_QR_timestamp 	peripherals[PERIPHERAL_XUFO_TIMESTAMP]
#define X32_display		peripherals[PERIPHERAL_DISPLAY]
#define SCALE_AE 12
#define MAX_AE 1023

//max and min values for motors

#define LIMIT_RATE 20

#define MAX_LIFT 100
#define MAX_ROLL 100
#define MAX_PITCH 100
#define MAX_YAW 100

#define MIN_LIFT 0
#define MIN_ROLL 0
#define MIN_PITCH 0
#define MIN_YAW 0

#define MAX_MOTOR1 200
#define MAX_MOTOR2 200
#define MAX_MOTOR3 200
#define MAX_MOTOR4 200

#define MIN_MOTOR1 0
#define MIN_MOTOR2 0
#define MIN_MOTOR3 0
#define MIN_MOTOR4 0

#define INITIAL_P_YAW 30
#define INITIAL_P1_FULL 10
#define INITIAL_P2_FULL 10
#define INITIAL_MODE SAFE

#define REDUCING_ROLL 1
#define REDUCING_PITCH 1
#define REDUCING_YAW 1
#define REDUCING_LIFT 1

//Controller Ps limits
#define UP_P_YAW 1
#define DOWN_P_YAW 1
#define MAX_P_YAW 100
#define MIN_P_YAW 1

#define UP_P1_FULL 1
#define DOWN_P1_FULL 1
#define MAX_P1_FULL 100
#define MIN_P1_FULL 1

#define UP_P2_FULL 1
#define DOWN_P2_FULL 1
#define MAX_P2_FULL 100
#define MIN_P2_FULL 1

//functions
void isr_timer(void);
void isr_qr_link(void);
void safe_mode_ctrl(void);
void panic_mode_ctrl(void);
void manual_mode_ctrl(void);
void calibration_mode(void);
void yaw_mode_ctrl(void);
void full_mode_ctrl(void);
void init_state(void);
void handleMode (void);
void handleInput (void);
void init_state(void);
void clip_RPYL(void);
void compute_RPMs(void);


//variables
extern comm_type mode, type;
extern int lift, roll, pitch, yaw;
extern int oo1, oo2, oo3, oo4;
extern int s0, s1, s2, s3, s4, s5;
extern int s0_bias, s1_bias, s2_bias, s3_bias, s4_bias, s5_bias;
extern int p_yaw, p1_full, p2_full;
extern volatile int finished;

#endif
