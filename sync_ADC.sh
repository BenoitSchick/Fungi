#!/bin/bash

mkdir -p /home/fr1boise/Documents/Fungi/ADC/AD777_code/Measurement_ADC/tmp

rsync -a --delete \
/home/fr1boise/Documents/Fungi/ADC/AD777_code/Measurement_ADC/ \
/home/fr1boise/Documents/Fungi/ADC/AD777_code/Measurement_ADC/tmp/


rclone copy /home/fr1boise/Documents/Fungi/ADC/AD777_code/Measurement_ADC/tmp gdrive:mesure_ADC \
  --checksum \
  --min-age 2m \
  --transfers=2 \
  --drive-use-trash=false
