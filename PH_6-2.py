#!/usr/bin/env python
# available: https://github.com/ShigekiKarita/practice/blob/master/PH_6-2.py

B = [0.6961663, 0.4079426, 0.8974794]
wavelength = [0.0684043, 0.1162414, 9.896161]  # micro-meter
c = 29.9792458 * 10 ** 8 * 10 ** 6  # micro-meter
pi = 3.141592653589793

def n(x):
    sigma = 0.
    for j in range(0, 3):
        sigma += B[j] * x ** 2 / (x ** 2 - wavelength[j])
    return (1 + sigma) ** 0.5

def gradient(f, h=1. / 10 ** 6):
    def fx(x):
        return (f(x + h) - f(x)) / h
    return fx

if __name__ == '__main__':
    nxx = gradient(gradient(n))  # 1 / micro-meter^2
    x = 1.550  # micro-meter
    D = - x / c * nxx(x)  # s / micro-meter^2
    beta_2 = - x ** 2 / (2 * pi * c) * D  # s / micro-meter
    L_max = (0.4 * 10**-10)**2 / abs(beta_2) * 10**-9  # kilo-meter

    print(D)
    print(beta_2)
    print(L_max)

    # 3rd-party libraries to plot figures
    import numpy as np
    import matplotlib.pyplot as plt

    xrange = np.linspace(0.5, 2.0, 100)
    fig = plt.figure(1)
    plt.subplots_adjust(hspace=0.5, wspace=0.5)

    plt.subplot(2, 2, 1)
    plt.plot(xrange, (lambda x: n(x) ** 2)(xrange))
    plt.ylabel("n^2")
    plt.xlabel("wavelength[um]")

    plt.subplot(2, 2, 2)
    plt.plot(xrange, n(xrange))
    plt.ylabel("n")
    plt.xlabel("wavelength[um]")

    plt.subplot(2, 2, 3)
    plt.plot(xrange, gradient(n)(xrange))
    plt.ylabel("n'[um^-1]")
    plt.xlabel("wavelength[um]")

    plt.subplot(2, 2, 4)
    plt.plot(xrange, nxx(xrange))
    plt.ylabel("n''[um^-2]")
    plt.xlabel("wavelength[um]")

    fig.savefig("ph.pdf")
