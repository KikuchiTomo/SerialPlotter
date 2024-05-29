import sys
import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
from scipy import signal

argv = sys.argv
argc = len(argv)

if argc < 2:
    print("bye")
    exit

fpath = argv[1]
print(fpath)

def butterLowpass(lowcut, fs, order=4):

    nyq = 0.5 * fs
    low = lowcut / nyq
    b, a = signal.butter(order, low, btype='low')
    return b, a


def lowpass(x, lowcut, fs, order=4):
    b, a = butterLowpass(lowcut, fs, order=order)
    y = signal.filtfilt(b, a, x)
    return y

def calcSamplingRate(a):
    x = a[0]
    n = 0.0
    for b in a[1:]:
        n += b - x
        x = b
    d = n / len(a) 
    return 1.0 / d

ds = pd.read_csv(fpath, header=None, names=['sq', 'pg', 'ps', 'tm', 'ch', 'vl'], sep=' ')
c0 = ds.query('ch == 1').to_numpy()
c1 = ds.query('ch == 2').to_numpy()
c2 = ds.query('ch == 3').to_numpy()

t0 = c0[:,3] / 1000.0
v0 = np.clip(1 - c0[:,5] / 4095.0, -1.0, 1.0) 
t1 = c1[:,3] / 1000.0
v1 = np.clip(1 - c1[:,5] / 4095.0, -1.0, 1.0)
t2 = c2[:,3] / 1000.0
v2 = np.clip(1 - c2[:,5] / 4095.0, -1.0, 1.0)

print(calcSamplingRate(t0))
print(calcSamplingRate(t1))
print(calcSamplingRate(t2))

print(t0, len(t0), len(v0))
print(t1, len(t1), len(v1))
print(t2, len(t2), len(v2))

a0 = np.mean(v0[0:500])
a1 = np.mean(v1[0:500])
a2 = np.mean(v2[0:500])

v0 = v0 - a0
v1 = v1 - a1
v2 = v2

fc = 15.0
v0 = lowpass(v0, fc, calcSamplingRate(t0))
v1 = lowpass(v1, fc, calcSamplingRate(t1))
v2 = lowpass(v2, fc, calcSamplingRate(t2))

m0 = np.max(np.abs(v0))
m1 = np.max(np.abs(v1))
m2 = np.max(np.abs(v2))

mx = np.max(np.array([m0, m1, m2]))

v0 = v0 / mx
v1 = v1 / mx
v2 = v2 / mx

fig = plt.figure()
ax0 = fig.add_subplot(3, 1, 1)
ax0.plot(t0, v0)

ax1 = fig.add_subplot(3, 1, 2)
ax1.plot(t1, v1)

ax2 = fig.add_subplot(3, 1, 3)
ax2.plot(t2, v2)


lx = [4830.0 , 4860.0]

ax0.set_xlim(lx)
ax1.set_xlim(lx)
ax2.set_xlim(lx)

plt.show()
