#!/bin/bash
sudo rm -rf /home/pi/BACKUP
sudo rm -rf /home/pi/vivent
sudo rm -rf /home/pi/vivent_DHCP
cp -rf BACKUP /home/pi
cp -rf vivent /home/pi
cp -rf vivent_DHCP /home/pi
chmod +x /home/pi/vivent/vivent.sh
chmod +x /home/pi/vivent/copy_stdout.sh
chmod +x /home/pi/vivent/vivent
chmod +x /home/pi/vivent_DHCP/vivent
chmod +x /home/pi/BACKUP/vivent_no_need_ADC_test_for_USB/vivent.sh
chmod +x /home/pi/BACKUP/vivent_no_need_ADC_test_for_USB/copy_stdout.sh
chmod +x /home/pi/BACKUP/vivent_no_need_ADC_test_for_USB/vivent
