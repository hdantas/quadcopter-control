#ifndef _KALMAN_H
#define _KALMAN_H

#define SCALE_KALMAN 4100
#define INV_C1 256
#define FIXED_CONV1 16 // 2^1048576
#define FIXED_CONV2 16 // 2^1048576
#define FIXED_CONV3 20 // 2^16777216

#define INITIAL_P2PHI 268
#define MAX_P2PHI 400
#define MIN_P2PHI 100
#define UP_P2PHI 5
#define DOWN_P2PHI 5

//variables
extern int p_bias_roll, p_kalman_roll, phi_kalman_roll, phi_error_roll, p2phi;
extern int q_bias_pitch, q_kalman_pitch, theta_kalman_pitch, theta_error_pitch;

//functions
void init_kalman(void);
void reset_kalman(void);
void kalman_filter_pitch(void);
void kalman_filter_roll(void);
#endif
