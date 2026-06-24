import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from datetime import datetime

CSV_FILE = "ttyACM0_merged.csv"
MAX_IMPEDANCE_LIMIT = 100000 

df = pd.read_csv(
    CSV_FILE,
    header=None,
    names=["freq", "impedance", "phase", "timestamp"]
)

df["timestamp"] = pd.to_datetime(
    df["timestamp"],
    format="%a %b %d %H:%M:%S %Y"
)
df["freq"] = pd.to_numeric(df["freq"])
df = df[df["freq"] <= 100000]

df["impedance"] = df["impedance"].clip(upper=MAX_IMPEDANCE_LIMIT)

df["new_sweep"] = df["freq"] == 1000.0
df["sweep_id"] = df["new_sweep"].cumsum()

sweep_times = df.groupby("sweep_id")["timestamp"].first()
Z = df.pivot_table(
    index="freq",
    columns="sweep_id",
    values="impedance"
)

Z = Z.sort_index(ascending=True)
freqs = Z.index.values
values = Z.values
print(Z.index)
print(df[df["freq"].between(14998, 16631)])
invalid_sweeps = Z.columns[Z.isna().all()]


# DETECTION LIGNE BLANCHE -----------------------------------------
# On cherche les colonnes (sweeps) qui contiennent des valeurs manquantes (NaN)
invalid_sweeps = Z.columns[Z.isna().any()]

if len(invalid_sweeps) > 0:
    print(f"{len(invalid_sweeps)} ligne(s) blanche(s) détectée(s) !")
    for s_id in invalid_sweeps:
        # On récupère le timestamp associé à ce sweep défectueux
        error_time = sweep_times.loc[s_id]
        print(f"Erreur détectée au sweep ID {s_id} le : {error_time}")
# -------------------------------------------------------------------------



freq_step = np.diff(freqs)[0] if len(freqs) > 1 else 100
y_boundaries = np.append(freqs, freqs[-1] + freq_step)

times = sweep_times.values
if len(times) > 1:
    time_step = np.median(np.diff(times))
    x_boundaries = np.append(times, times[-1] + time_step)
else:
    x_boundaries = np.append(times, times[-1] + np.timedelta64(1, 'm'))

plt.figure(figsize=(14, 8))

mesh = plt.pcolormesh(
    x_boundaries,
    y_boundaries,
    values,
    shading="flat",
    cmap="viridis",
    vmin=1000,
    vmax=MAX_IMPEDANCE_LIMIT
)

cbar = plt.colorbar(mesh)
cbar.set_label("Impedance (Ohms)")

plt.ylabel("Fréquence (Hz)", fontsize=12)
plt.xlabel("Date", fontsize=12)

plt.ylim(freqs.min(), freqs.max())
plt.yticks(freqs, [f"{int(f)}" for f in freqs])

plt.gca().xaxis.set_major_locator(mdates.DayLocator(interval=1))
plt.gca().xaxis.set_major_formatter(mdates.DateFormatter('%d/%m'))

plt.title("ttyACM0 - 3 juin au 24 juin")
plt.grid(axis='x', linestyle='--', alpha=0.4)
plt.tight_layout()
plt.show()
