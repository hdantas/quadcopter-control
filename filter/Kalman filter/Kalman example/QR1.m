% Author: Enrico Caruso

%--------------------------------------------------------------------------
% QR1 -- 
%--------------------------------------------------------------------------
% load data
	load logfile.dat;
	t = logfile(:,1);
	say = logfile(:,2);
	sax = logfile(:,3);
	saz = logfile(:,4);
	sp = logfile(:,5);
	sq = logfile(:,6);
	sr = logfile(:,7);
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
% do fft on phi
%--------------------------------------------------------------------------    
	f = (0:n-1)*(fs/n);
	fftphi = fft(phi);
	Pphi = fftphi.*conj(fftphi)/n;
	figure; plot(f(1:n/2),Pphi(1:n/2));    
    
%--------------------------------------------------------------------------    
% show p and phi, integrate p to phi to show movement vs noise
%--------------------------------------------------------------------------    
% p2phi is given
    p2phi = 0.0041;
% compute phi from p
    phi_p(1:n) = 0;
    phi_p(1) = 0;
    for i=(2:n) phi_p(i) = phi_p(i-1) + p(i-1) * p2phi; end;
    figure;
    plot(t,phi,'r');
    hold on;
    plot(t,p,'b');
    hold on;
    plot(t,phi_p,'g');

%--------------------------------------------------------------------------    
% filter phi to reduce scale to show movement
%--------------------------------------------------------------------------    
% filter 2nd order 10Hz 
    a2 = [1,-1.9289423,0.9313817];
    b2 = [0.0006098548,0.0012197096,0.0006098548];
    phi2 = filter(b2,a2,phi);
    p2 = filter(b2,a2,p);
    figure; plot(t,[phi2,p,phi_p']);

% compute phi from p2
    phi_p2(1:n) = 0;
    phi_p2(1) = 0;
    for i=(2:n) phi_p2(i) = phi_p2(i-1) + p2(i-1) * p2phi; end;
    figure; plot(t,[phi2,p2,phi_p2']);
% show that phi_p2 equals phi_p (so no need to filter p)
    figure; plot(t,[phi_p',phi_p2']);

    
