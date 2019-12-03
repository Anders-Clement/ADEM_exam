
import matplotlib.pyplot as plt
import numpy as np
from sympy import *


n_precision = 50
num_of_steps = 100


t, n = symbols('t n')

input_func = -2*t**2
T = 2*np.pi       #time period (2pi for trigonometric functions)

a0 = (1/np.pi) * integrate(input_func, (t, -T/2, T/2))
print(a0)
an = (2/T)*integrate(input_func*cos((2*n*np.pi*t)/T), (t,-T/2, T/2))
print(an)
bn = (2/T)*integrate(input_func*sin((2*n*np.pi*t)/T), (t,-T/2, T/2))
print(bn)



fourier = an*cos((2*n*np.pi*t)/T) + bn*sin((2*n*np.pi*t)/T)#((-2*pi)/n)*(cos(n*pi))*(cos(n*t))

#a0:
y_eq = a0/2    #-4.0*(pi**2/3)

N = range(1, n_precision)

for i in range(1, len(N)):
    y_eq += fourier.subs(n, i)


X = np.linspace(-T/2, T/2, num_of_steps)
Y = []
Y1 = []

for i in range(0, len(X)):
    tNow = X[i]
    Y.append(y_eq.subs(t, tNow))
    Y1.append(input_func.subs(t, tNow))#-2*(tNow**2))

#print(Y)

plt.plot(X, Y1, label='y(t)', color="red")
plt.plot(X, Y, label='app', color="blue")

plt.xlabel('Time sec')
plt.ylabel('Height m')

plt.title("Signals")
plt.grid(linestyle='-', linewidth=0.5)

plt.legend()

plt.show()
