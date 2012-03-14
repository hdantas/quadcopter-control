	#include <stdlib.h>
	#include <stdio.h>
	#define FILE_NAME "data1.dat"
	#define lenght_max 35000
	#define C1 256
    #define C2 1000000
    #define p2phi 0.0041
    #define phi_b 515
    #define theta_b 523
    #define p_b 414


	int main () {

	    FILE *fp, *fpout;
	    int phi[lenght_max], theta[lenght_max], p[lenght_max], saz,sq,sr;
	    float phi_kalman[lenght_max], p_bias[lenght_max],p_kalman[lenght_max],phi_error[lenght_max];
	    int i,n;
	    int phi_kalman_int[lenght_max], p_kalman_int[lenght_max], p_bias_int[lenght_max], phi_error_int[lenght_max];


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

	    printf("read %d lines",i);
	    n=i;
	    p_bias[0] = 0;
        phi_kalman[0] = 0;
        p_kalman[0]=0;
        phi_error[0] = 0;

	    for (i=1;i<n;i++) {
            p_kalman[i] = p[i-1] - p_bias[i-1];
            phi_kalman[i] = phi_kalman[i-1] + p_kalman[i] * p2phi;
            phi_error[i] = phi_kalman[i] - phi[i];
            phi_kalman[i] = phi_kalman[i] - phi_error[i] / C1;
            p_bias[i] = p_bias[i-1] + (phi_error[i]/p2phi)/ C2;
	    }

        p_bias_int[0] = 0;
        phi_kalman_int[0] = 0;
        p_kalman_int[0]=0;
        phi_error_int[0] = 0;

//	    for (i=1;i<n;i++) {
//            p_kalman_int[i] = p[i-1] - p_bias_int[i-1];
//            phi_kalman[i]_int= phi_kalman_int[i-1] + p_kalman_int[i] * p2phi;
//            phi_error_int[i] = phi_kalman_int[i] - phi[i];
//            phi_kalman_int[i] = phi_kalman_int[i] - phi_error_int[i] / C1;
//            p_bias_int[i] = p_bias_int[i-1] + (phi_error_int[i]/p2phi)/ C2;
//	    }

	    for (i=1;i<n;i++) {
            p_kalman_int[i] = 4100*p[i-1] - p_bias_int[i-1];
            phi_kalman_int[i]= phi_kalman_int[i-1] + (p_kalman_int[i] * 4300)/1048576;
            phi_error_int[i] = phi_kalman_int[i] - 4100*phi[i];
            phi_kalman_int[i] = phi_kalman_int[i] - (phi_error_int[i] *4096)/1048576;
            p_bias_int[i] = p_bias_int[i-1] + (phi_error_int[i]*  1023)/4194304;
	    }

	    for (i=0;i<n;i++) {
            p_kalman_int[i]=p_kalman_int[i]/4100;
            phi_kalman_int[i]=phi_kalman_int[i]/4100;
            p_bias_int[i] = p_bias_int[i]/4100;
	    }


        for (i=0;i<n;i++) {
            fprintf(fpout,"%d %f %f %f %d %d %d\n",phi[i],phi_kalman[i],p_kalman[i],p_bias[i],phi_kalman_int[i],p_kalman_int[i],p_bias_int[i]);
	    }

//        for (i=0;i<n;i++) {
//            fprintf(fpout,"%d %f %f %f %f %f %f\n",phi[i],phi_kalman[i],p_kalman[i],p_bias[i],phi_kalman_int[i],p_kalman_int[i],p_bias_int[i]);
//	    }

	    fclose(fp);
        fclose(fpout);

	    return EXIT_SUCCESS;

	    }
