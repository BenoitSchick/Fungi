import numpy as np
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from datetime import datetime
from scipy.signal import medfilt
from scipy.ndimage import uniform_filter1d

timestamps_raw = np.fromfile("timestamp", dtype=np.int64)
global_mask = timestamps_raw > 0
timestamps = timestamps_raw[global_mask]

# -------- Décimation --------
max_points = 100000 
step = max(1, len(timestamps) // max_points)
timestamps_decimated = timestamps[::step]


# Conversion des timestamps en objets datetime pour l'axe X
dates = [datetime.fromtimestamp(ts) for ts in timestamps_decimated]
num_time_steps = len(timestamps_decimated)

# Init matrice (8 canaux x nb_data)
channels_to_keep = [0, 2, 3, 4, 5, 6, 7]
num_channels = len(channels_to_keep)
data_matrix = np.full((num_channels, num_time_steps), np.nan)

print(f"Décimation appliquée (1 point sur {step})")

# Traitement des 8 canaux 
for idx, ch in enumerate(channels_to_keep):
    filename = f"channel{ch}"
    try:
        values_raw = np.fromfile(filename, dtype=np.int32)
    except FileNotFoundError:
        print(f"Fichier {filename} introuvable, passé.")
        continue

    # Alignement avec timestamp 
    min_len = min(len(values_raw), len(timestamps_raw))
    values = values_raw[:min_len]
    values = values[global_mask[:min_len]]
    
    # Même décimation que timestamp 
    values = values[::step]
    current_len = len(values)

    # -------- Filtrage --------
    kernel_size = 1001 
    values_filtered = medfilt(values, kernel_size=kernel_size)
    window_size = 200 
    values_filtered = uniform_filter1d(values_filtered, size=window_size)

    data_matrix[idx, :current_len] = values_filtered


# --------- PLOT ----------------------
plt.figure(figsize=(14, 6))

x_lims = mdates.date2num(dates)
y_centers = np.arange(num_channels)
mesh = plt.pcolormesh(x_lims, y_centers, data_matrix, cmap='viridis', shading='nearest', vmin=-10000, vmax=10000)

ax = plt.gca()
ax.xaxis.set_major_formatter(mdates.DateFormatter('%Y-%m-%d %H:%M:%S'))
ax.xaxis.set_major_locator(mdates.DayLocator(interval=1)) 
plt.gcf().autofmt_xdate()

ax.set_yticks(range(num_channels))
ax.set_yticklabels([f"Canal {ch}" for ch in channels_to_keep])
# ax.set_ylabel("Canaux")

cbar = plt.colorbar(mesh, ax=ax, orientation='vertical', pad=0.02)
# cbar.set_label('Valeur ADC')

plt.title("Spectrogramme des mesures électrophysiologiques (Canaux 0 à 7)")
ax.grid(True, axis='x', color='white', linestyle='--', alpha=0.4, linewidth=0.8, zorder=3)

plt.tight_layout()
plt.show()
