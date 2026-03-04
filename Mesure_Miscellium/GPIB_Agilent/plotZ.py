import numpy as np
import matplotlib.pyplot as plt


fstart = 40      # Hz
fstop = 4e6      # Hz
filename = "Mesure_freqCoupure/Electrode_Tiges/AvecMisc_apresColo/meas_Z_H5L11.txt"

with open(filename, "r") as f:
    file_meas = f.read()

mag_meas, phase_meas = file_meas.split("###############")


mag = np.array([float(x) for x in mag_meas.replace("\n", "").split(",") if x.strip()])
phase = np.array([float(x) for x in phase_meas.replace("\n", "").split(",") if x.strip()])
mag = mag[::2]
phase = phase[::2]

pts = len(mag)
f = np.logspace(np.log10(fstart), np.log10(fstop), pts)

# print(mag)
# print("\n \n")
# print(phase)

fig, (ax1, ax2) = plt.subplots(2, 1, sharex=True)

plt.suptitle(filename)

ax1.semilogx(f, 20 * np.log10(np.abs(mag)))
ax1.set_ylabel("Magnitude (dB)")
ax1.grid(True, which="both")

ax2.semilogx(f, phase)
ax2.set_ylabel("Phase (deg)")
ax2.set_xlabel("Frequency (Hz)")
ax2.grid(True, which="both")

plt.tight_layout()
plt.show()
