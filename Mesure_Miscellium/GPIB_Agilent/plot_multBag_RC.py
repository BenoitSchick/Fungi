import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.axes_grid1.inset_locator import inset_axes

# Paramètres
fstart = 40      # Hz
fstop = 4e6      # Hz
filenames = [
# With ECG Electrodes                                                                                    
    # "Mesure_freqCoupure/Electrode_Patch/SansMisc/meas_RC_H8L2.txt",
    # "Mesure_freqCoupure/Electrode_Patch/SansMisc/meas_RC_H8L5.txt",
    # "Mesure_freqCoupure/Electrode_Patch/SansMisc/meas_RC_H8L11.txt",
    # "Mesure_freqCoupure/Electrode_Patch/SansMisc/meas_RC_H5L11.txt",
   
    # "Mesure_freqCoupure/Electrode_Patch/AvecMisc_avantColo/meas_RC_H8L2.txt",
    # "Mesure_freqCoupure/Electrode_Patch/AvecMisc_avantColo/meas_RC_H8L5.txt",
    # "Mesure_freqCoupure/Electrode_Patch/AvecMisc_avantColo/meas_RC_H8L11.txt",
    # "Mesure_freqCoupure/Electrode_Patch/AvecMisc_avantColo/meas_RC_H5L11.txt",
    
    # "Mesure_freqCoupure/Electrode_Patch/AvecMisc_apresColo/meas_RC_H8L2.txt",
    # "Mesure_freqCoupure/Electrode_Patch/AvecMisc_apresColo/meas_RC_H8L5.txt",
    # "Mesure_freqCoupure/Electrode_Patch/AvecMisc_apresColo/meas_RC_H8L11.txt",
    # "Mesure_freqCoupure/Electrode_Patch/AvecMisc_apresColo/meas_RC_H5L11.txt",

    # With Metal Rod Electrodes
    "Mesure_freqCoupure/Electrode_Tiges/SansMisc/meas_RC_H8L2.txt",
    "Mesure_freqCoupure/Electrode_Tiges/SansMisc/meas_RC_H8L5.txt",
    "Mesure_freqCoupure/Electrode_Tiges/SansMisc/meas_RC_H8L11.txt",
    "Mesure_freqCoupure/Electrode_Tiges/SansMisc/meas_RC_H5L11.txt",

    #"Mesure_freqCoupure/Electrode_Tiges/AvecMisc_apresColo/meas_RC_H8L2.txt",
    # "Mesure_freqCoupure/Electrode_Tiges/AvecMisc_apresColo/meas_RC_H8L5.txt",
    # "Mesure_freqCoupure/Electrode_Tiges/AvecMisc_apresColo/meas_RC_H8L11.txt",
    # "Mesure_freqCoupure/Electrode_Tiges/AvecMisc_apresColo/meas_RC_H5L11.txt",
]

fig, (ax1, ax2) = plt.subplots(2, 1, sharex=True, figsize=(10, 6))

for filename in filenames:
    with open(filename, "r") as f:
        file_meas = f.read()
    
    Cs_f, Rs_f = file_meas.split("###############")
    
    Cs = np.array([float(x) for x in Cs_f.replace("\n", "").split(",") if x.strip()])
    Rs = np.array([float(x) for x in Rs_f.replace("\n", "").split(",") if x.strip()])
    
    Cs = Cs[::2]
    Rs = Rs[::2]
    
    pts = len(Cs)
    f = np.logspace(np.log10(fstart), np.log10(fstop), pts)
    
    ax1.semilogx(f, Cs, label=filename.split("/")[-1])
    ax2.semilogx(f, Rs, label=filename.split("/")[-1])

# Labels et grille
ax1.set_ylabel("Capacitance $C_s$ (F)")
ax1.grid(True, which="both")
ax1.legend()

ax2.set_ylabel("Resistance $R_s$ ($\Omega$)")
ax2.set_xlabel("Frequency (Hz)")
ax2.grid(True, which="both")
ax2.legend()


# plt.suptitle("ECG Electrodes - Substrate only")                                                 
# plt.suptitle("ECG Electrodes - Substrate with mycelium in colonization phase")
# plt.suptitle("ECG Electrodes - Substrate fully colonized by mycelium")

plt.suptitle("Metal Rod Electrodes - Substrate only")                                                 
# plt.suptitle("Metal Rod Electrodes - Substrate fully colonized by mycelium")

# ----- ZOOM -----
zoom_min = 1e4
zoom_max = 1e6

# Zoom pour Cs
axins1 = inset_axes(ax1, width="40%", height="40%", loc="upper right")
for filename in filenames:
    with open(filename, "r") as f:
        file_meas = f.read()
    Cs_f, _ = file_meas.split("###############")
    Cs = np.array([float(x) for x in Cs_f.replace("\n", "").split(",") if x.strip()])[::2]
    f = np.logspace(np.log10(fstart), np.log10(fstop), len(Cs))
    mask = (f >= zoom_min) & (f <= zoom_max)
    axins1.semilogx(f[mask], Cs[mask], label=filename.split("/")[-1])
axins1.set_xlim(zoom_min, zoom_max)
#axins1.set_ylim(Cs[mask].min(), Cs[mask].max())
axins1.grid(True, which="both")
axins1.minorticks_on()

# Zoom pour Rs
axins2 = inset_axes(ax2, width="40%", height="40%", loc="upper right")
for filename in filenames:
    with open(filename, "r") as f:
        file_meas = f.read()
    _, Rs_f = file_meas.split("###############")
    Rs = np.array([float(x) for x in Rs_f.replace("\n", "").split(",") if x.strip()])[::2]
    f = np.logspace(np.log10(fstart), np.log10(fstop), len(Rs))
    mask = (f >= zoom_min) & (f <= zoom_max)
    axins2.semilogx(f[mask], Rs[mask], label=filename.split("/")[-1])
axins2.set_xlim(zoom_min, zoom_max)
#axins2.set_ylim(Rs[mask].min(), Rs[mask].max())
axins2.grid(True, which="both")
axins2.minorticks_on()


ax1.legend(loc='upper center', bbox_to_anchor=(0.5, 0), ncol=4)
#ax2.legend(loc='upper center', bbox_to_anchor=(0.5, -0.1), ncol=4)
#plt.tight_layout()
plt.show()



