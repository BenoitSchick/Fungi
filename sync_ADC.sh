#!/bin/bash

mkdir -p /tmp/adc_snapshot

rsync -a --delete \
/home/fr1boise/Documents/Fungi/ADC/AD777_code/Measurement_ADC/ \
/tmp/adc_snapshot/

rclone copy /tmp/adc_snapshot gdrive:mesure_ADC \
  --checksum \
  --min-age 2m \
  --transfers=2 \
  --drive-use-trash=false
