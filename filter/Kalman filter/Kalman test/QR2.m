%--------------------------------------------------------------------------
% QR2 -- 
%--------------------------------------------------------------------------
% load data
	load data1.dat;
	t = 1: size(data1,1);
	phi = data1(:,1)-515;
	theta= data1(:,2)-523;
	p = data1(:,4)-414;
	n = length(t);

%--------------------------------------------------------------------------    
% kalman filter to remove dynamic offset in p using phi
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

    
