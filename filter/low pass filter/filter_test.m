% 1st-order Butterworth - fc 10 Hz at fs 1000 Hz
% Author: Enrico Caruso
load LPFout.dat
Fs = 1000;	%sample frequency
%filter coefficent
a=[1, -0.939062505817492];
b=[0.0304687470912538, 0.0304687470912538];
t=LPFout(:,1);
%signal to filter
x=LPFout(:,2);
%filter with matlab function
ym=filter(b,a,x);
%floating point output of the filter written in c
yf=LPFout(:,3);
%integer output of the filter written in c
yi=LPFout(:,5);

%500 is a scale factor (fixed point)
plot(t,500*ym,'b');
hold on;
plot(t,500*yf,'r');
hold on;
plot(t,yi,'g');
hold on;
plot(t,500*x,'c');
