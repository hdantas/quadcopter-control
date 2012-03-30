#ifndef _FILTER_H
#define _FILTER_H

#define SCALE_SHIFT_KALMAN 12 //4096
#define INV_C1 350 //256 old value
#define INV_C1_P2PHI 268
#define FIXED_CONV1 16 // 2^1048576
#define FIXED_CONV2 16 // 2^1048576
#define FIXED_CONV3 20 // 2^16777216

#define INITIAL_P2PHI 268
#define MAX_P2PHI 400
#define MIN_P2PHI 100
#define UP_P2PHI 5
#define DOWN_P2PHI 5

#define FIXED_CONV3 20

#define FIXED_CONV4 6 // 2^64
#define FIXED_CONV5 6 // 2^64
#define FIXED_CONV6 10 // 2^1024
#define B0 998
#define B1 998
#define A1 961

//variables
extern int p_bias_roll, p_kalman_roll, phi_kalman_roll, phi_error_roll, p2phi;
extern int q_bias_pitch, q_kalman_pitch, theta_kalman_pitch, theta_error_pitch;
extern int s5_filtered, s5_old, s5_filtered_old;

//functions
void init_kalman(void);
void reset_kalman(void);
void kalman_filter_pitch(void);
void kalman_filter_roll(void);
void butterworth_filter (void);
void init_butterworth(void);
#endif
