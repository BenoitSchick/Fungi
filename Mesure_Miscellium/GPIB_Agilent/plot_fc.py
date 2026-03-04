import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.axes_grid1.inset_locator import inset_axes


fstart = 40      # Hz
fstop = 4e6      # Hz
filename = "Mesure_freqCoupure/Electrode_Tiges/AvecMisc_apresColo/meas_RC_H8L2.txt"

with open(filename, "r") as f:
    file_meas = f.read()

Cs_f, Rs_f = file_meas.split("###############")


Cs = np.array([float(x) for x in Cs_f.replace("\n", "").split(",") if x.strip()])
Rs = np.array([float(x) for x in Rs_f.replace("\n", "").split(",") if x.strip()])
Cs = Cs[::2]
Rs = Rs[::2]

pts = len(Cs)
f = np.logspace(np.log10(fstart), np.log10(fstop), pts)

f_c = 1/(2*np.pi*Rs*Cs)

# print(Cs)
# print("\n \n")
# print(Rs)

fig, ax1 = plt.subplots()
fig.suptitle(filename)

ax1.semilogx(f, f_c)
ax1.set_xlabel("Fréquence (Hz)")
ax1.set_ylabel("Fréquence de coupure (Hz)")
ax1.grid(True, which="both")

zoom_min = 40
zoom_max = 1e3
mask = (f >= zoom_min) & (f <= zoom_max)

axins1 = inset_axes(ax1, width="40%", height="40%", loc="center right")
axins1.semilogx(f[mask], f_c[mask])
axins1.set_xlim(zoom_min, zoom_max)
axins1.set_ylim(f_c[mask].min(), f_c[mask].max())
axins1.minorticks_on()
axins1.grid(True, which="both")

plt.show()


