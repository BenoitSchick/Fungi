import numpy as np
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from datetime import datetime

values = np.fromfile("channel3", dtype=np.int32)
timestamps = np.fromfile("timestamp", dtype=np.int64)

# Sécurité : Si jamais il y a un léger décalage de fin de fichier
min_len = min(len(values), len(timestamps))
values = values[:min_len]
timestamps = timestamps[:min_len]

# 2. Conversion rapide des timestamps en objets datetime
dates = [datetime.fromtimestamp(ts) for ts in timestamps]

# 3. Création du graphique
plt.figure(figsize=(12, 6))
plt.plot(dates, values, label="Signal ADC (Channel 0)", color='b', linewidth=1)

# Formatage de l'axe X pour afficher les heures/dates proprement
plt.gca().xaxis.set_major_formatter(mdates.DateFormatter('%Y-%m-%d %H:%M:%S'))
plt.gca().xaxis.set_major_locator(mdates.AutoDateLocator())
plt.gcf().autofmt_xdate() # Incline les étiquettes pour éviter les chevauchements

# Habillage
plt.title("Mesures de l'ADC au cours du temps", fontsize=14, fontweight='bold')
plt.xlabel("Temps (Date / Heure)", fontsize=12)
plt.ylabel("Valeur ADC", fontsize=12)
plt.grid(True, linestyle='--', alpha=0.5)
plt.legend(loc="upper right")

# Affichage
plt.tight_layout()
plt.show()
