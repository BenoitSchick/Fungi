import numpy as np
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from datetime import datetime

# Chargement unique des timestamps (commun à tous les canaux)
timestamps_raw = np.fromfile("timestamp", dtype=np.int64)

plt.figure(figsize=(14, 7))

# Boucle pour charger et afficher chaque canal de 0 à 7
for ch in range(8):
    if ch == 1:
        continue
    filename = f"channel{ch}"
    try:
        values_raw = np.fromfile(filename, dtype=np.int32)
    except FileNotFoundError:
        print(f"Fichier {filename} introuvable, passé.")
        continue

    # Alignement des tailles
    min_len = min(len(values_raw), len(timestamps_raw))
    values = values_raw[:min_len]
    timestamps = timestamps_raw[:min_len]

    # Suppression des timestamps invalides (0)
    mask = timestamps > 0
    values = values[mask]
    timestamps = timestamps[mask]

    # -------- Décimation --------
    max_points = 100000
    step = max(1, len(values) // max_points)
    values = values[::step]
    timestamps = values_timestamps = timestamps[::step] # Correction locale pour la boucle

    # Conversion datetime (générée une seule fois ou à chaque fois selon l'alignement)
    dates = [datetime.fromtimestamp(ts) for ts in timestamps]

    # Ajout de la courbe sur le graphique
    plt.plot(dates, values, linewidth=0.8, label=f"channel {ch}")

print(f"Décimation appliquée (1 point sur {step}). Graphique en cours de génération...")

# -------- Configuration du Graphique --------
plt.gca().xaxis.set_major_formatter(
    mdates.DateFormatter('%Y-%m-%d %H:%M:%S')
)
plt.gca().xaxis.set_major_locator(
    mdates.DayLocator(interval=1)
)
plt.gcf().autofmt_xdate()

# Limitation de l'axe Y à 10 000
# plt.ylim(top=10000) 
# Note : si tu veux aussi bloquer le bas à 0, utilise : plt.ylim(0, 10000)

plt.title("Mesures électrophysiologiques du 3 juin au 18 juin (Canaux 0 à 7)")
plt.xlabel("Temps")
plt.ylabel("Valeur ADC")
plt.grid(True, linestyle='--', alpha=0.5)

plt.tight_layout()
plt.legend(loc="upper right") # Placement de la légende pour 8 courbes
plt.show()
