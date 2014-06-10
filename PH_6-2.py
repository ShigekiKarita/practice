#!/usr/bin/env python


B = [0.6961663, 0.4079426, 0.8974794]
wavelength = [0.0684043, 0.1162414, 9.896161]
c = 29.9792458 * 10**8 * 10**6
pi = 3.141592653589793

def n(x):
    sigma = 0.
    for j in range(0, 3):
        sigma += B[j] * x**2 / (x**2 - wavelength[j])
    return (1 + sigma)**0.5

def gradient(f, h=1./10**2):
    def fx(x):
        return (f(x + h) - f(x)) / h
    return fx


if __name__ == '__main__':
    nxx = gradient(gradient(n))
    x = 1.550
    D = - x / c * nxx(x)
    beta_2 = - x**2 / (2 * pi * c) * D
    print(beta_2)


    # follow 3rd libraries to plot figures
    import numpy as np
    import matplotlib.pyplot as plt

    xrange = np.linspace(0.5, 2.0, 100)

    fig = plt.figure(1)
    plt.subplots_adjust(hspace=0.5)

    plt.subplot(2, 2, 1)
    plt.plot(xrange, (lambda x : n(x)**2)(xrange))
    plt.xlabel("n^2")

    plt.subplot(2, 2, 2)
    plt.plot(xrange, n(xrange))
    plt.xlabel("n")

    plt.subplot(2, 2, 3)
    plt.plot(xrange, gradient(n)(xrange))
    plt.xlabel("n'")

    plt.subplot(2, 2, 4)
    plt.plot(xrange, nxx(xrange))
    plt.xlabel("n''")

    fig.savefig("ph.png")

