load data.dat;
t = 1: size(data,1);
phi = data(:,1)-515;
n = length(t);
a= [1,-1.9289423,0.9313817];
b = [0.0006098548,0.0012197096,0.0006098548];
phi3 = filter(b,a,phi);

x1=0;
x2=0;
y1=0;
y2=0;
phi2=zeros(n,1);

for i=1:n
    phi2(i,1)=b(1)*phi(i,1)+b(2)*x1+b(3)*x2-a(2)*y1-a(3)*y2;
    x2=x1;
    x1=phi(i,1);
    y2=y1;
    y1=phi2(i,1);
end
