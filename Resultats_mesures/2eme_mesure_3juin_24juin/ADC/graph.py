import numpy as np
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from datetime import datetime
from scipy.signal import medfilt
from scipy.ndimage import uniform_filter1d

values = np.fromfile("channel7", dtype=np.int32)
timestamps = np.fromfile("timestamp", dtype=np.int64)

min_len = min(len(values), len(timestamps))
values = values[:min_len]
timestamps = timestamps[:min_len]

# Suppression des timestamps invalides (0)
mask = timestamps > 0
values = values[mask]
timestamps = timestamps[mask]

# -------- Décimation --------
max_points = 100000
step = max(1, len(values) // max_points)
values = values[::step]
# values_filtered = values_filtered[::step]
timestamps = timestamps[::step]

print(f"Décimation: 1 point sur {step}")
print(f"Points affichés : {len(values)}")

# -------- Median Filter --------
kernel_size = 1001 
values_filtered = medfilt(values, kernel_size=kernel_size)

# -------- Moyenne glissante --------
window_size = 200 
values_filtered = uniform_filter1d(values_filtered, size=window_size)


# --------- Plot des graphes --------
dates = [datetime.fromtimestamp(ts) for ts in timestamps]
plt.figure(figsize=(12, 6))

# Valeur filtre
plt.plot(dates,values_filtered, linewidth=0.8, label="channel7")

# Valeur non filtre
kernel_size = 15 
values_nonfilt = medfilt(values, kernel_size=kernel_size)
plt.plot(dates,values_nonfilt, linewidth=0.8, label="channel7 non filtré")

plt.gca().xaxis.set_major_formatter(
    mdates.DateFormatter('%Y-%m-%d %H:%M:%S')
)
plt.gca().xaxis.set_major_locator(
    mdates.DayLocator(interval=1)
)
plt.gcf().autofmt_xdate()
plt.ylim(-10000, 10000)
plt.title("Mesures électrophysiologiques du 3 juin au 24 juin")
# plt.xlabel("Temps")
plt.ylabel("Valeur ADC")
plt.grid(True, linestyle='--', alpha=0.5)

plt.tight_layout()
plt.legend()
plt.show()
