# Plot spectrum of wav file saved by SDR# as function of time (3D plot)
# Bandwidth = 32 kHz when input sample rate = 2.048 MHz (decimated by 64)
# Asgaut Eng/2016-10-02

import numpy as np
import scipy.io.wavfile as wav
import scipy.signal as signal
from scipy.fftpack import fft, fftfreq, fftshift
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

fs, data = wav.read("satnogs_7663458_2023-06-03T20-58-52_BISONSAT.wav", mmap=True)

T = 1/fs # sample spacing
N = int(0.1 / T) # FFT bin size = 10 Hz

print("Sample rate %f MS/s" % (fs / 1e6))
print("Num samples %d" % len(data))
print("Duration: %f s" % (T * len(data)))
iq = np.empty(N, np.complex64)

Nd = N // 8 // 8
Td = T * 8 * 8
print("Decimated sample rate: %f S/s" % (1 / Td))

xf = fftshift(fftfreq(Nd, Td)) # FFT bin frequencies
yf = np.linspace(0, 90, 90) # from 0 to 90 s in 90 steps
zf = np.empty((len(yf), len(xf)), float) # matrix for FFT magnitudes
for i in range(0, len(yf), 1):
    index = int(yf[i] * fs)
    iq.real, iq.imag = data[index:index+N,0], data[index:index+N,1]
    iqd = signal.decimate(iq, 8, zero_phase=False)
    iqd = signal.decimate(iqd, 8, zero_phase=False)
    zf[i] = 1./Nd * np.abs(fft(iqd))
    zf[i] = fftshift(zf[i])

fig = plt.figure()
ax = Axes3D(fig)
X, Y = np.meshgrid(xf, yf)
Z = zf
ax.plot_surface(X, Y, Z, rstride=1, cstride=1, cmap=plt.cm.hot)
plt.show()

