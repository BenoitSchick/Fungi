from datetime import datetime
import matplotlib.dates as mdates
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

# =====================================================================
# 1. PRÉPARATION DES DONNÉES DE LA COURBE 1 (Channel 7)
# =====================================================================
values_c1 = np.fromfile("channel7", dtype=np.int32)
timestamps_c1 = np.fromfile("timestamp", dtype=np.int64)

min_len = min(len(values_c1), len(timestamps_c1))
values_c1 = values_c1[:min_len]
timestamps_c1 = timestamps_c1[:min_len]

# Suppression des timestamps invalides (0)
mask = timestamps_c1 > 0
values_c1 = values_c1[mask]
timestamps_c1 = timestamps_c1[mask]

# Décimation
max_points = 100000
step = max(1, len(values_c1) // max_points)
values_c1 = values_c1[::step]
timestamps_c1 = timestamps_c1[::step]

# Conversion datetime pour la Courbe 1
dates_c1 = [datetime.fromtimestamp(ts) for ts in timestamps_c1]


# =====================================================================
# 2. PRÉPARATION DES DONNÉES DE LA COURBE 2 (Heatmap Impédance)
# =====================================================================
CSV_FILE = "mesure_impedance_3juin_18juin/merged/ttyACM0_merged.csv"
MAX_IMPEDANCE_LIMIT = 30000

df = pd.read_csv(
    CSV_FILE, header=None, names=["freq", "impedance", "phase", "timestamp"]
)

df["timestamp"] = pd.to_datetime(
    df["timestamp"], format="%a %b %d %H:%M:%S %Y"
)
df["freq"] = pd.to_numeric(df["freq"])
df = df[df["freq"] <= 100000]
df["impedance"] = df["impedance"].clip(upper=MAX_IMPEDANCE_LIMIT)

df["new_sweep"] = df["freq"] == 1000.0
df["sweep_id"] = df["new_sweep"].cumsum()

sweep_times = df.groupby("sweep_id")["timestamp"].first()
Z = df.pivot_table(
    index="freq", columns="sweep_id", values="impedance"
)

Z = Z.sort_index(ascending=True)
freqs = Z.index.values
values_c2 = Z.values

freq_step = np.diff(freqs)[0] if len(freqs) > 1 else 100
y_boundaries = np.append(freqs, freqs[-1] + freq_step)

times = sweep_times.values
if len(times) > 1:
    time_step = np.median(np.diff(times))
    x_boundaries = np.append(times, times[-1] + time_step)
else:
    x_boundaries = np.append(times, times[-1] + np.timedelta64(1, 'm'))


# =====================================================================
# 3. GÉNÉRATION DU GRAPHIQUE SUPERPOSÉ
# =====================================================================
fig, ax1 = plt.subplots(figsize=(14, 8))

# --- Tracé du Spectrogramme (Courbe 2) sur l'axe principal (Gauche) ---
mesh = ax1.pcolormesh(
    x_boundaries,
    y_boundaries,
    values_c2,
    shading="flat",
    cmap="viridis",
    vmin=1000,
    vmax=MAX_IMPEDANCE_LIMIT,
    alpha=0.85,  # Légère transparence pour mieux voir la courbe par-dessus
)

cbar = fig.colorbar(mesh, ax=ax1, pad=0.08)
cbar.set_label("Impedance (Ohms)", fontsize=11)

ax1.set_ylabel("Fréquence (Hz)", fontsize=12)
ax1.set_ylim(freqs.min(), freqs.max())
ax1.set_yticks(freqs)
ax1.set_yticklabels([f"{int(f)}" for f in freqs])


# --- Tracé de Channel 7 (Courbe 1) sur le second axe (Droite) ---
ax2 = ax1.twinx()
ax2.plot(
    dates_c1,
    values_c1,
    linewidth=0.9,
    color="crimson",
    label="channel7 (ADC)",
)

ax2.set_ylabel("Valeur ADC", fontsize=12, color="crimson")
ax2.tick_params(axis="y", labelcolor="crimson")
ax2.set_ylim(-10000,10000)  


# --- Configuration commune de l'axe X (Date) ---
ax1.xaxis.set_major_locator(mdates.DayLocator(interval=1))
ax1.xaxis.set_major_formatter(mdates.DateFormatter("%d/%m"))
fig.autofmt_xdate()

ax1.set_xlabel("Date", fontsize=12)
plt.title(
    "Spectrogramme d'impédance (ttyACM0) & Signal Électrophysiologique (CH7) - Mesure du 3juin au 18juin",
    fontsize=14,
    fontweight="bold",
)

# Grille basée sur l'axe principal
ax1.grid(axis="x", linestyle="--", alpha=0.4)

# Légende pour la courbe en ligne
ax2.legend(loc="upper right")

plt.tight_layout()
plt.show()
