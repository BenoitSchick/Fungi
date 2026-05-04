import numpy as np

# data = np.fromfile("Measurement_ADC/channel0", dtype=np.int32)
data = np.fromfile("Measurement_ADC/timestamp", dtype=np.int32)

print(data[:20])  # afficher les 20 premières valeurs
