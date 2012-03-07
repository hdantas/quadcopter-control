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
#define X32_QR_s0 		peripherals[PERIPHERAL_XUFO_S0]
#define X32_QR_s1 		peripherals[PERIPHERAL_XUFO_S1]
#define X32_QR_s2 		peripherals[PERIPHERAL_XUFO_S2]
#define X32_QR_s3 		peripherals[PERIPHERAL_XUFO_S3]
#define X32_QR_s4 		peripherals[PERIPHERAL_XUFO_S4]
#define X32_QR_s5 		peripherals[PERIPHERAL_XUFO_S5]
#define X32_QR_timestamp 	peripherals[PERIPHERAL_XUFO_TIMESTAMP]
#define X32_display		peripherals[PERIPHERAL_DISPLAY]

//max and min values for motors
#define MAX_LIFT 0
#define MIN_LIFT 128
#define MAX_ROLL 64
#define MIN_ROLL -64
#define MAX_PITCH 64
#define MIN_PITCH -64
#define MAX_YAW 64
#define MIN_YAW -64

//rates for decreasing motor RPM and minimum value for motor RPM for panic mode
#define MIN_MOTOR1 20
#define MIN_MOTOR2 20
#define MIN_MOTOR3 20
#define MIN_MOTOR4 20
#define REDUCING_RPM1 5
#define REDUCING_RPM2 5
#define REDUCING_RPM3 5
#define REDUCING_RPM4 5


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
void handleInput (comm_type type);

//variables
extern int lift, roll, pitch, yaw;
extern int oo1, oo2, oo3, oo4;
extern int s0, s1, s2, s3, s4, s5;
extern int p_yaw;
extern comm_type mode, type;
extern volatile int finished;
extern int yaw_error;

#endif