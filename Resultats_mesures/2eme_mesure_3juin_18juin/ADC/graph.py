import numpy as np
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from datetime import datetime

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
timestamps = timestamps[::step]

print(f"Décimation: 1 point sur {step}")
print(f"Points affichés : {len(values)}")

# Conversion datetime
dates = [datetime.fromtimestamp(ts) for ts in timestamps]

# Graphique
plt.figure(figsize=(12, 6))
plt.plot(dates, values, linewidth=0.8, label="channel7")

plt.gca().xaxis.set_major_formatter(
    mdates.DateFormatter('%Y-%m-%d %H:%M:%S')
)
plt.gca().xaxis.set_major_locator(
    mdates.DayLocator(interval=1)
)
plt.gcf().autofmt_xdate()

plt.title("Mesures électrophysiologiques du 3 juin au 18 juin")
plt.xlabel("Temps")
plt.ylabel("Valeur ADC")
plt.grid(True, linestyle='--', alpha=0.5)

plt.tight_layout()
plt.legend()
plt.show()



