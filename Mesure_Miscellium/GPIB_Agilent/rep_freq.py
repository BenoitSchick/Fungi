import numpy as np
import matplotlib.pyplot as plt


def rc_frequency_response(R, C, fstart, fstop, n_points=1000, mode="Z1"):
    f = np.logspace(np.log10(fstart), np.log10(fstop), n_points)
    w = 2 * np.pi * f

    if mode == "Z1": # R et C en série
        H = (1j * w * R * C + 1) / (1j * w * C)
    elif mode == "Z2": # R et C en parallèle
        H = R / (1 + 1j * w * R * C)
    else:
        raise ValueError("mode must be 'Z1' or 'Z2'")

    magnitude = 20 * np.log10(np.abs(H))
    phase = np.angle(H, deg=True)

    return f, magnitude, phase


R = 8e3      # ohms
C = 1.5e-7     # farads
fstart = 0.1   # Hz
fstop = 10e6  # Hz

f, mag, ph = rc_frequency_response(R, C, fstart, fstop, mode="Z1")

plt.suptitle("Impédance d'un RC parallèle théorique en fonction de f")

plt.subplot(2,1,1)
plt.semilogx(f, mag)
plt.xlabel("Frequency (Hz)")
plt.ylabel("Magnitude (dB)")
plt.grid(True)

plt.subplot(2,1,2)
plt.semilogx(f, ph)
plt.xlabel("Frequency (Hz)")
plt.ylabel("Phase (deg)")
plt.grid(True)
plt.tight_layout()
plt.show()
