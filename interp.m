x = [30,100];

y = [25,180];

a = [0:200];

p = polyfit (x, y, 1);

b = polyval(p, a);

plot(x,y,a,b);
