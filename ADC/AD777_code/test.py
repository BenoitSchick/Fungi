import numpy as np
import time

SAMPLES_PER_TIMESTAMP = 500
values = np.fromfile("Measurement_ADC/channel0", dtype=np.int32)
timestamps = np.fromfile("Measurement_ADC/timestamp", dtype=np.int64)

print(f"Valeurs lues      : {len(values)}")
print(f"Timestamps lus    : {len(timestamps)}")


nblocks = min(len(timestamps), len(values)

for b in range(nblocks):

    ts = timestamps[b]

    date_str = time.strftime(
        "%Y-%m-%d %H:%M:%S",
        time.localtime(ts)
    )

    start = b * SAMPLES_PER_TIMESTAMP
    end   = start + SAMPLES_PER_TIMESTAMP

    bloc = values[start:end]

    print(f"=== {date_str} ===")
    print(bloc)
