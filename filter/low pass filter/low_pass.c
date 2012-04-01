#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#define FILE_NAME "data1.dat"
#define lenght_max 35000
#define f 10
#define Fs 1000

/* Author: Enrico Caruso */

#define SCALE_SHIFT_BUTTERWORTH 9
#define FIXED_CONV4 6 // 2^32768
#define FIXED_CONV5 6 // 2^32768
#define FIXED_CONV6 10 // 2^1024
//fixed point coefficients
#define B0 998
#define B1 998
#define A1 961

//floating point coefficients
float a[2]={1, -0.9390625};
float b[2]={0.0304687, 0.0304687};
// xi i-th sample to filter
// xi_old previous sample
// yi i-th filtered sample
// yi_old previous filtered sample

//float variables
float xi, xi_old, yi, yi_old;
//integer variables
int xi_int, xi_old_int, yi_int, yi_old_int;

void filter_float (void) {
    yi=b[0]*xi + b[1]*xi_old - a[1]*yi_old;
}

void filter_int (void) {
//FIXED_CONV contains also the scale values of 512
    yi_int=((B0*xi_int) >> FIXED_CONV4) + ((B1*xi_old_int) >> FIXED_CONV5) + ((A1*yi_old_int)>> FIXED_CONV6);
}


int main () {
    FILE *fpout;
    //vectors with all the samples and all the outputs of the filter
    int x_int[lenght_max], y_int[lenght_max];
    float x[lenght_max], y[lenght_max], t[lenght_max];
    int i;


    if ((fpout=fopen("LPFout.dat","w"))==NULL) {
        printf("Unable to create the file LPFout.dat");
        exit(EXIT_FAILURE);
    }

    t[0]=0;

    for (i=1; i<1000; i++) {
        t[i]=t[i-1]+(float) 1/Fs;
    }

//sin signal
    for (i=0; i<1000; i++) {
        x[i]=500*sin(2*2.14*f*t[i]);
        x_int[i]=(int) x[i];
    }

//initialization of previous output
    y[0]=0;
    y_int[0]=0;

    for (i=1; i<1000; i++) {
        xi_old=x[i-1];
        yi_old=y[i-1];
        xi_old_int=x_int[i-1];
        yi_old_int=y_int[i-1];
        xi=x[i];
        xi_int=x_int[i];
        filter_float();
        filter_int();
        y[i]=yi;
        y_int[i]=yi_int;
    }

    for (i=0;i<1000;i++) {
        fprintf(fpout,"%f %f %f %d %d\n", t[i], x[i], y[i], x_int[i], y_int[i] );
    }

    fclose(fpout);

    return EXIT_SUCCESS;

}
