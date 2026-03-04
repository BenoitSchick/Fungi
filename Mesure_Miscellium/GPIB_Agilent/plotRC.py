import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.axes_grid1.inset_locator import inset_axes


fstart = 40      # Hz
fstop = 4e6      # Hz
filename = "Mesure_freqCoupure/Electrode_Tiges/AvecMisc_apresColo/meas_RC_H5L11.txt"

with open(filename, "r") as f:
    file_meas = f.read()

Cs_f, Rs_f = file_meas.split("###############")


Cs = np.array([float(x) for x in Cs_f.replace("\n", "").split(",") if x.strip()])
Rs = np.array([float(x) for x in Rs_f.replace("\n", "").split(",") if x.strip()])
Cs = Cs[::2]
Rs = Rs[::2]

pts = len(Cs)
f = np.logspace(np.log10(fstart), np.log10(fstop), pts)

# print(Cs)
# print("\n \n")
# print(Rs)

fig, (ax1, ax2) = plt.subplots(2, 1, sharex=True)

plt.suptitle(filename)

ax1.semilogx(f, Cs)
ax1.set_ylabel("Capacité Cs (F)")
ax1.grid(True, which="both")

ax2.semilogx(f, Rs)
ax2.set_ylabel("Résistance Rs ($\Omega$)")
ax2.set_xlabel("Frequency (Hz)")
ax2.grid(True, which="both")



# ZOOM 
zoom_min = 1e4
zoom_max = 4e6
mask = (f >= zoom_min) & (f <= zoom_max)

axins1 = inset_axes(ax1, width="40%", height="40%", loc="upper right")
axins1.semilogx(f[mask], Cs[mask])
axins1.set_xlim(zoom_min, zoom_max)
axins1.set_ylim(Cs[mask].min(), Cs[mask].max())
axins1.minorticks_on()
axins1.grid(True, which="both")

axins2 = inset_axes(ax2, width="40%", height="40%", loc="upper right")
axins2.semilogx(f[mask], Rs[mask])
axins2.set_xlim(zoom_min, zoom_max)
axins2.set_ylim(Rs[mask].min(), Rs[mask].max())
axins2.minorticks_on()
axins2.grid(True, which="both")

#plt.tight_layout()
plt.show()


