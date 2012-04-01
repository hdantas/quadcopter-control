#include <stdlib.h>
#include <stdio.h>
#include "kalman.h"

/* Author: Enrico Caruso */

#define FILE_NAME "data2.dat"
#define lenght_max 35000
#define C1 187
#define C2 1000000
#define P2PHI 0.0041
// bias of sensors
#define phi_b 515
#define theta_b 523
#define p_b 414

int s0, s1, s2, s3, s4;	//i-th value of the sensors
int phi[lenght_max], theta[lenght_max], p[lenght_max], saz,sq,sr;	//vector of the sensor values
float phi_kalman_float[lenght_max], p_bias_float[lenght_max],p_kalman_float[lenght_max],phi_error_float[lenght_max];	//vector of estimated state of QR with kalman (float)
int i,n;
int phi_kalman_int[lenght_max], p_kalman_int[lenght_max], p_bias_int[lenght_max], phi_error_int[lenght_max];	//vector of estimated state of QR with kalman (int)

//variable i-th used on the kalman function
extern int q_bias_pitch, q_kalman_pitch, theta_kalman_pitch, theta_error_pitch;
extern int p_bias_roll, p_kalman_roll,phi_kalman_roll,phi_error_roll;

extern int q_bias_pitch, q_kalman_pitch, theta_kalman_pitch, theta_error_pitch;
extern int p_bias_roll, p_kalman_roll,phi_kalman_roll,phi_error_roll;
extern int p2phi;

int main () {
	FILE *fp, *fpout;

//initialization of all the variable used in kalman filter
	init_kalman();

	i=0;

	if ((fp=fopen(FILE_NAME,"r"))==NULL) {
		printf("Unable to open the file %s",FILE_NAME);
		exit(EXIT_FAILURE);
	}

	if ((fpout=fopen("out.dat","w"))==NULL) {
		printf("Unable to create the file out.data");
		exit(EXIT_FAILURE);
	}

	while (fscanf(fp,"%d %d %d %d %d %d\n",&phi[i],&theta[i],&saz, &p[i], &sq, &sr)!=EOF) {
		phi[i]=phi[i]-phi_b;
		theta[i]=theta[i]-theta_b;
		p[i]=p[i]-p_b;
		i++;
	}

	printf("read %d lines\n",i);
	n=i;
	p_bias_float[0] = 0;
	phi_kalman_float[0] = 0;
	p_kalman_float[0]=0;
	phi_error_float[0] = 0;

//floating implementation of kalman filter
	for (i=1;i<n;i++) {
	    p_kalman_float[i] = p[i-1] - p_bias_float[i-1];
	    phi_kalman_float[i] = phi_kalman_float[i-1] + p_kalman_float[i] * P2PHI;
	    phi_error_float[i] = phi_kalman_float[i] - phi[i];
	    phi_kalman_float[i] = phi_kalman_float[i] - phi_error_float[i] / C1;
	    p_bias_float[i] = p_bias_float[i-1] + (phi_error_float[i]/P2PHI)/ C2;
	}

	p_bias_int[0] = 0;
	phi_kalman_int[0] = 0;
	p_kalman_int[0]=0;
	phi_error_int[0] = 0;

	reset_kalman();

//integer implementation of kalman filter
	for (i=0;i<n;i++) {
		s3=p[i];
		s0=phi[i];
		kalman_filter_roll();
		p_kalman_int[i]=p_kalman_roll;
            	phi_kalman_int[i]=phi_kalman_roll;
            	p_bias_int[i] = p_bias_roll;
	}



        for (i=0;i<n;i++) {
//we work in fixed point so we need to divide and moltiply our variable by some scale factor to obtain confrontable values (<<7, >>5 *128)
            fprintf(fpout,"%d %d %f %f %f %d %d %d\n",(phi[i]<<7),(p[i]<<7),(phi_kalman_float[i]*128),(p_kalman_float[i]*128),(p_bias_float[i]*128),(phi_kalman_int[i]>>5),(p_kalman_int[i]>>5),(p_bias_int[i]>>5));
	    }

	fclose(fp);
	fclose(fpout);
	return EXIT_SUCCESS;

}
