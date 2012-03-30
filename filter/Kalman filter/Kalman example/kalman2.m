%--------------------------------------------------------------------------
% QR2 -- 
%--------------------------------------------------------------------------
% load data
	load sensors_data_1.dat;
	%t = sensors_data_1(:,1);
    t=1:size(sensors_data_1,1);
    
	say = sensors_data_1(:,1);
	sax = sensors_data_1(:,2);
	saz = sensors_data_1(:,3);
	sp = sensors_data_1(:,4);
	sq = sensors_data_1(:,5);
	sr = sensors_data_1(:,6);
% sample freq: t is in us
	fs = length(t)*1000000/(t(length(t))-t(1));
	n = length(t);
% calibrate static offset
    ay_off = say(n);
	ax_off = sax(n);
	az_off = saz(n);
	p_off = sp(n);
	q_off = sq(n);
	r_off = sr(n);
% estimate real signal values
	ay = say - ay_off;
	ax = sax - ax_off;
	az = saz - az_off;
	p = (sp - p_off);  
	q = - (sq - q_off);
	r = sr - r_off;
% map acc to angles (simplified) 
    phi = ay;
    theta = ax;
    
%--------------------------------------------------------------------------    
% filter phi to reduce scale to show movement
%--------------------------------------------------------------------------    
% filter 2nd order 10Hz 
    a2 = [1,-1.9289423,0.9313817];
    b2 = [0.0006098548,0.0012197096,0.0006098548];
    phi2 = filter(b2,a2,phi);

%--------------------------------------------------------------------------    
% kalman filter to remove dynamic offset in p using phi2
%--------------------------------------------------------------------------    
    p2phi = 0.0041;
    p_bias(1:n) = 0;
    phi_kalman(1:n) = 0;
    C1 = 256;
    C2 = 1000000;
    for i=(2:n)
        p_kalman(i) = p(i-1) - p_bias(i-1);
        phi_kalman(i) = phi_kalman(i-1) + p_kalman(i) * p2phi;
        phi_error(i) = phi_kalman(i) - phi(i);
        phi_kalman(i) = phi_kalman(i) - phi_error(i) / C1;
        p_bias(i) = p_bias(i-1) + (phi_error(i)/p2phi) / C2;
    end;
    figure; 
    plot(t,phi,'r');
    hold on;
    plot(t,p,'b');
    hold on;
    plot(t,phi_kalman,'g');
    figure; plot(t,p_bias');
    
    
    
% compare phi from kalman with phi from p
    phi_p(1:n) = 0;
    phi_p(1) = 0;
    for i=(2:n) 
        phi_p(i) = phi_p(i-1) + p(i-1) * p2phi; 
    end;
    figure; plot(t,phi_p,t,phi_kalman);

    