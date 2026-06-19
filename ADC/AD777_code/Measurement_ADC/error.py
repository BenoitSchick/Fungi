import numpy as np
import os

DIR = "/home/fr1boise/Documents/Fungi/ADC/AD777_code/Measurement_ADC"
file_path = os.path.join(DIR, "error")

# lecture du binaire (int32 little-endian)
data = np.fromfile("error", dtype="<i4")

print("Nombre de valeurs :", len(data))
print("Aperçu :", data[:20])
