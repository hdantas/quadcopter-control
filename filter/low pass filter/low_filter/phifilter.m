% Author: Enrico Caruso

clear all;
load filterphi.dat;
phi=filterphi(:,1);
phi2=filterphi(:,2);
phi2_int=filterphi(:,3);
a= [1,-1.9289423,0.9313817];
b = [0.0006098548,0.0012197096,0.0006098548];
phi3 = filter(b,a,phi);
figure;
plot(phi3);
hold all;
plot(phi2);
figure;
plot(phi2);
hold all;
plot(phi2_int);
