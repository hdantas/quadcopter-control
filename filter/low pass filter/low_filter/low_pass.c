#include <stdlib.h>
#include <stdio.h>
#define FILE_NAME "data1.dat"
#define lenght_max 35000
#define phi_b 515
#define scale 200


int main () {
    FILE *fp, *fpout;
    int phi[lenght_max], theta, p, saz, sq, sr;
    float phi2[lenght_max];
    float a[3]={1,-1.9289423,0.93138176};
    float b[3]={0.0006098548,0.0012197096,0.0006098548};
    int i,n;
    float x1,x2,y1,y2;
    //int phi_kalman_int[lenght_max], p_kalman_int[lenght_max], p_bias_int[lenght_max], phi_error_int[lenght_max];

    int x1_int,x2_int,y1_int,y2_int;
    int phi2_int[lenght_max];

    i=0;

    if ((fp=fopen(FILE_NAME,"r"))==NULL) {
        printf("Unable to open the file %s",FILE_NAME);
        exit(EXIT_FAILURE);
    }

    if ((fpout=fopen("filterphi.dat","w"))==NULL) {
        printf("Unable to create the file filterphi.data");
        exit(EXIT_FAILURE);
    }

    while (fscanf(fp,"%d %d %d %d %d %d\n",&phi[i],&theta,&saz, &p, &sq, &sr)!=EOF) {
        phi[i]=phi[i]-phi_b;
        i++;
    }

    printf("read %d lines",i);
    n=i;
    x1=0;
    x2=0;
    y1=0;
    y2=0;

    for (i=0;i<n;i++) {
        phi2[i]=(b[0]*phi[i])+(b[1]*x1)+(b[2]*x2)-(a[1]*y1)-(a[2]*y2);
        x2=x1;
        x1=phi[i];
        y2=y1;
        y1=phi2[i];
    }

    x1_int=0;
    x2_int=0;
    y1_int=0;
    y2_int=0;

    for (i=0;i<n;i++) {
        phi2_int[i]=(scale*phi[i]*10232)/16777216+(x1_int*20463)/16777216+(x2_int*10232)/16777216-(-63208*y1_int)/32768-(30520*y2_int)/32768;
        x2_int=x1_int;
        x1_int=scale*phi[i];
        y2_int=y1_int;
        y1_int=phi2_int[i];
    }




//    for (i=0;i<n;i++) {
//        phi2_int[i]=(50*phi[i]*10232)/16777216+(x1_int*20463)/16777216+(x2_int*10232)/16777216-(-32362282*y1_int)/16777216-(15625992*y2_int)/16777216;
//        x2_int=x1_int;
//        x1_int=50*phi[i];
//        y2_int=y1_int;
//        y1_int=phi2_int[i];
//    }




    for (i=0;i<n;i++) {
        phi2_int[i]=phi2_int[i]/scale;
    }

    for (i=0;i<n;i++) {
        fprintf(fpout,"%d %f %d\n",phi[i],phi2[i],phi2_int[i]);
    }


    fclose(fp);
    fclose(fpout);

    return EXIT_SUCCESS;

    }
