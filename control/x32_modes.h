#ifndef _X32_MODES_H
#define _X32_MODES_H

#define X32_timer1_per		peripherals[PERIPHERAL_TIMER1_PERIOD]
#define X32_timer2_per		peripherals[PERIPHERAL_TIMER2_PERIOD]
#define X32_leds		peripherals[PERIPHERAL_LEDS]
#define X32_ms_clock		peripherals[PERIPHERAL_MS_CLOCK]
#define X32_us_clock		peripherals[PERIPHERAL_US_CLOCK]
#define X32_QR_a0		peripherals[PERIPHERAL_XUFO_A0]
#define X32_QR_a1		peripherals[PERIPHERAL_XUFO_A1]
#define X32_QR_a2		peripherals[PERIPHERAL_XUFO_A2]
#define X32_QR_a3		peripherals[PERIPHERAL_XUFO_A3]
#define X32_QR_s0		peripherals[PERIPHERAL_XUFO_S0] //sax
#define X32_QR_s1		peripherals[PERIPHERAL_XUFO_S1] //say
#define X32_QR_s2		peripherals[PERIPHERAL_XUFO_S2] //saz
#define X32_QR_s3		peripherals[PERIPHERAL_XUFO_S3] //sp
#define X32_QR_s4		peripherals[PERIPHERAL_XUFO_S4] //sq
#define X32_QR_s5		peripherals[PERIPHERAL_XUFO_S5] //sr
#define X32_QR_timestamp	peripherals[PERIPHERAL_XUFO_TIMESTAMP]
#define X32_display		peripherals[PERIPHERAL_DISPLAY]

//#define SCALE_AE 14
//#define MAX_AE 1023

#define SCALE_MANUAL 14
#define SCALE_YAW_ERROR 1
#define SCALE_ROLL_ERROR 50
#define SCALE_PITCH_ERROR 50

#define SCALE_LIFT 30
#define SCALE_ROLL 156
#define SCALE_PITCH 156
#define SCALE_YAW 1

//max and min values for motors

#define LIMIT_RATE 100

//#define MAX_LIFT 100
//#define MAX_ROLL 100
//#define MAX_PITCH 100
//#define MAX_YAW 100

//#define MIN_LIFT 0
//#define MIN_ROLL 0
//#define MIN_PITCH 0
//#define MIN_YAW 0

#define MAX_MOTOR1 1023
#define MAX_MOTOR2 1023
#define MAX_MOTOR3 1023
#define MAX_MOTOR4 1023

#define MIN_MOTOR1 10
#define MIN_MOTOR2 10
#define MIN_MOTOR3 10
#define MIN_MOTOR4 10

#define INITIAL_P_YAW 40
#define INITIAL_P1_FULL 10
#define INITIAL_P2_FULL 25
#define INITIAL_MODE SAFE

#define MIN_MOTORS 0
#define REDUCING_RPM 10


//Controller Ps limits
#define UP_P_YAW 1
#define DOWN_P_YAW 1
#define MAX_P_YAW 100
#define MIN_P_YAW 1

#define SHIFT_PHI_KALMAN_ROLL 5
#define SHIFT_P_KALMAN_ROLL 5
#define SHIFT_THETA_KALMAN_PITCH 5
#define SHIFT_Q_KALMAN_PITCH 5

#define UP_P1_FULL 1
#define DOWN_P1_FULL 1
#define MAX_P1_FULL 100
#define MIN_P1_FULL 1

#define UP_P2_FULL 1
#define DOWN_P2_FULL 1
#define MAX_P2_FULL 100
#define MIN_P2_FULL 1


#define BLINK_COUNT 1000		// 1000ms = 1s
#define TIME_OUT_QR_LINK 1500	// 1500us = 1.5ms
#define TIME_OUT_PC_LINK 100	// 100ms

//functions
void isr_timer1(void);
void isr_timer2(void);
void isr_qr_link(void);
void isr_overflow(void);
void isr_division_by_zero(void);
void isr_trap(void);
void isr_out_of_memory(void);

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
void clip_AE(void);
void check_pc_link(void);
void check_qr_link(void);
void toggle_led(int i);


//variables
extern comm_type mode, type;
extern int lift, roll, pitch, yaw;
extern int lift_error, roll_error, pitch_error, yaw_error;
extern int oo1, oo2, oo3, oo4;
extern int s0, s1, s2, s3, s4, s5;
extern int s0_bias, s1_bias, s2_bias, s3_bias, s4_bias, s5_bias;
extern unsigned int p_yaw, p1_full, p2_full;
extern volatile int finished, terminate;
extern unsigned int time_last_packet;

#endif
