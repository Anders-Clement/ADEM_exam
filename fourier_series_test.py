
import matplotlib.pyplot as plt
import numpy as np
from sympy import *


n_precision = 20
x_steps = 100


t, n = symbols('t n')

input_func = -2*t**2
T = np.pi       #time period (2pi for trigonometric functions)


input_func = input_func.subs(t, (T*t)/(2*np.pi))
print("Calculating coefficients")
a0 = (1/np.pi) * integrate(input_func, (t, -T/2, T/2))
print("a0: ",end="")
print(a0)
an = (2/T)*integrate((input_func*cos(t)/T), (t,-T/2, T/2))
print("an: ",end="")
print(an)
bn = (2/T)*integrate((input_func*sin(t)/T), (t,-T/2, T/2))
print("bn: ",end="")
print(bn)
print("")

fourier = an*cos((2*n*np.pi*t)/T) + bn*sin((2*n*np.pi*t)/T)

for i in range(1,n_precision):
    n_precision = i

    #a0:
    y_eq = a0/2

    N = range(1, n_precision)
    #add n_precision amount of sines/cosines:
    for i in range(1, len(N)):
        y_eq += fourier.subs(n, i)

    #arrays for holding plot data
    X = np.linspace(-T/2, T/2, x_steps)
    Y = []
    Y1 = []

    #calculate graph for current fourier series
    for i in range(0, len(X)):
        Y.append(y_eq.subs(t, X[i]))
        Y1.append(input_func.subs(t, X[i]))

    #print(Y)
    fig = plt.gcf()
    fig.clf()
    plt.plot(X, Y1, label='y(t)', color="red")
    plt.plot(X, Y, label='app', color="blue")

    plt.xlabel('Time sec')
    plt.ylabel('Height m')

    plt.title("Signals")
    plt.grid(linestyle='-', linewidth=0.5)

    plt.legend()

    print("Precision with n=", end="")
    print(n_precision)
    plt.pause(1)

print("")
print("Final result")
plt.show()
