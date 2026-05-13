import numpy as np
import time

SAMPLES_PER_TIMESTAMP = 125 
values = np.fromfile("Measurement_ADC/channel1", dtype=np.int32)
timestamps = np.fromfile("Measurement_ADC/timestamp", dtype=np.int64)

n = len(values)

for i in range(n):
    date_str = time.strftime("%Y-%m-%d %H:%M:%S",time.localtime(timestamps[i]))
    print(f"Date: {date_str} | Valeur: {values[i]}")


print(f"Valeurs lues      : {len(values)}")
print(f"Timestamps lus    : {len(timestamps)}")
