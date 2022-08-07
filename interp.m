# Ligado
x = [30,90];
y = [25,180];

# Desligado
#x = [30,100];
#y = [25,100];

a = [0:240];

p = polyfit (x, y, 1);

b = polyval(p, a);

plot(x,y,a,b);
