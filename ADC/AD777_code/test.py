import numpy as np

data = np.fromfile("channel0", dtype=np.int32)

print(data[:20])  # afficher les 20 premières valeurs
