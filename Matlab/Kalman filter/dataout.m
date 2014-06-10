clear all;
% Author: Enrico Caruso
load out.dat;
fs=1266.46595714178; %sample frequency
n=length(out);
t=0:1/fs:(n-1)/fs;
%sensor values
phi=out(:,1);
p=out(:,2);
%kalman floating point
phi_kalman=out(:,3);
p_kalman=out(:,4);
p_bias=out(:,5);
%kalman integer
phi_kalman_int=out(:,6);
p_kalman_int=out(:,7);
p_bias_int=out(:,8);
figure;
plot(t,phi);
hold all;
plot(t,phi_kalman);
figure;
plot(t,phi_kalman);
hold all;
plot(t,phi_kalman_int);
figure;
plot(t,p_bias);
hold all;
plot(t,p_bias_int);
figure;
plot(t,phi);
hold all;
plot(t,phi_kalman_int);
