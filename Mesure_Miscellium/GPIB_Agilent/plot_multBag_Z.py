import numpy as np
import matplotlib.pyplot as plt

fstart = 40      # Hz
fstop = 4e6      # Hz

filenames = [

# With ECG Electrodes

    # "Mesure_freqCoupure/Electrode_Patch/SansMisc/meas_Z_H8L2.txt",
    # "Mesure_freqCoupure/Electrode_Patch/SansMisc/meas_Z_H8L5.txt",
    # "Mesure_freqCoupure/Electrode_Patch/SansMisc/meas_Z_H8L11.txt",
    # "Mesure_freqCoupure/Electrode_Patch/SansMisc/meas_Z_H5L11.txt",
   
    # "Mesure_freqCoupure/Electrode_Patch/AvecMisc_avantColo/meas_Z_H8L2.txt",
    # "Mesure_freqCoupure/Electrode_Patch/AvecMisc_avantColo/meas_Z_H8L5.txt",
    # "Mesure_freqCoupure/Electrode_Patch/AvecMisc_avantColo/meas_Z_H8L11.txt",
    # "Mesure_freqCoupure/Electrode_Patch/AvecMisc_avantColo/meas_Z_H5L11.txt",
    
    # "Mesure_freqCoupure/Electrode_Patch/AvecMisc_apresColo/meas_Z_H8L2.txt",
    # "Mesure_freqCoupure/Electrode_Patch/AvecMisc_apresColo/meas_Z_H8L5.txt",
    # "Mesure_freqCoupure/Electrode_Patch/AvecMisc_apresColo/meas_Z_H8L11.txt",
    # "Mesure_freqCoupure/Electrode_Patch/AvecMisc_apresColo/meas_Z_H5L11.txt",

# With Metal Rod Electrodes
    # "Mesure_freqCoupure/Electrode_Tiges/SansMisc/meas_Z_H8L2.txt",
    # "Mesure_freqCoupure/Electrode_Tiges/SansMisc/meas_Z_H8L5.txt",
    # "Mesure_freqCoupure/Electrode_Tiges/SansMisc/meas_Z_H8L11.txt",
    # "Mesure_freqCoupure/Electrode_Tiges/SansMisc/meas_Z_H5L11.txt",
    
    "Mesure_freqCoupure/Electrode_Tiges/AvecMisc_apresColo/meas_Z_H8L2.txt",
    "Mesure_freqCoupure/Electrode_Tiges/AvecMisc_apresColo/meas_Z_H8L5.txt",
    "Mesure_freqCoupure/Electrode_Tiges/AvecMisc_apresColo/meas_Z_H8L11.txt",
    "Mesure_freqCoupure/Electrode_Tiges/AvecMisc_apresColo/meas_Z_H5L11.txt",
]

plt.figure()

for filename in filenames:
    with open(filename, "r") as f:
        file_meas = f.read()

    mag_meas, phase_meas = file_meas.split("###############")

    mag = np.array([float(x) for x in mag_meas.replace("\n", "").split(",") if x.strip()])
    mag = mag[::2]

    pts = len(mag)
    f = np.logspace(np.log10(fstart), np.log10(fstop), pts)

    plt.semilogx(f, 20 * np.log10(np.abs(mag)), label=filename.split("/")[-1])


# plt.title("Metal Rod Electrodes - Substrate only")
# plt.title("Metal Rod Electrodes - Substrate with mycelium in colonization phase")
plt.title("Metal Rod Electrodes - Substrate fully colonized by mycelium")
plt.xlabel("Frequency (Hz)")
plt.ylabel("Magnitude (dB)")
plt.grid(True, which="both")
plt.legend()
plt.tight_layout()
plt.show()









































