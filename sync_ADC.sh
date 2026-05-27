#!/bin/bash

mkdir -p /home/fr1boise/Documents/Fungi/adc_snapshot

rsync -a --delete --inplace \
/home/fr1boise/Documents/Fungi/ADC/AD777_code/Measurement_ADC/ \
/home/fr1boise/Documents/Fungi/adc_snapshot


rclone copy /home/fr1boise/Documents/Fungi/adc_snapshot gdrive:mesure_ADC \
  --transfers=4 \
  --fast-list \
  --drive-use-trash=false
