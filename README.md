# Projet Fungi

The goal of this project is to measure the impedance of bags containing mycelium in order to observe how mycelium growth influences impedance. With this information, we aim to determine whether the mycelium is fully developed in the bags or if there are areas with little or no growth.

## Operation


The idea is to measure the impedance of the mycelium over several weeks to observe its evolution. Each measurement consists of a frequency sweep from 1 kHz to 100 kHz, taken every 5 minutes.  

The measurements from the Evaluation Board (EVAL-AD5940BIOZ) are retrieved via UART by a Raspberry Pi. The Raspberry Pi then synchronizes this data with Google Drive, making the measurements accessible from anywhere.

The measurement process is triggered by the Raspberry Pi. The reason for this is that up to four boards can be placed on the same bag, and the Raspberry Pi coordinates the measurements from each board to prevent interference.


## Material used
- Precision Impedance Analyzer - Agilent 4294A
- ECG Electrodes
- 4 x Bio-Electric Evaluation Board - EVAL-AD5940BIOZ
- Raspberry pi model 3B+

## Installation / Configuration
### Flashing the Evaluation Board


The first step is to install the Keil IDE to compile and program the Evaluation Board by following this setup guide: https://wiki.analog.com/resources/eval/user-guides/eval-ad5940/tools/keil_setup_guide  

The code used to program the evaluation board is located in the following folder: _Fungi/04_Software/EVAL-AD590ELCZ_Pain/ad5940-examples-master/ad5940-examples-master/examples/AD5940_BIA_

This code is based on the official repository: https://github.com/analogdevicesinc/ad5940-examples  

With this code, the evaluation board performs a frequency sweep to measure impedance only when it receives a trigger signal on **pin GP4** from the Raspberry Pi.

### Raspberry Pi Configuration

The Raspberry Pi runs two Python scripts in the background, located in:  
_Fungi/04_Software/EVAL-AD590ELCZ_Pain/python_Code_

- com_ports.py 
- Measurement_synchronization.py  

The first script retrieves data from each COM port to which the four AD5940 boards are connected. It stores the data in CSV files, one per COM port, in the folder: _Fungi/04_Software/EVAL-AD590ELCZ_Pain/python_Code/Measurement_

The second script sends trigger pulses to the **GP4 pins** of the four evaluation boards to start measurements. It also coordinates measurements between the boards to avoid interference. Additionally, it synchronizes the data in the `Measurement` folder with a `measure_impedance` folder on Google Drive using `rclone`.

#### Python Installation

- Install Python 3 and pip:

```bash
sudo apt update && sudo apt upgrade -y
sudo apt install python3 python3-pip -y
```
- Create and activate a virtual environment:

```bash
python3 -m venv mon_env
source mon_env/bin/activate
```

- Install required libraries:
```bash
# Library for serial communication
pip install pyserial
# Library to control Raspberry Pi GPIO
pip install RPi.GPIO
```

#### Google Drive Remote Configuration
To synchronize the local `Measurement` folder with a remote folder on Google Drive, install and configure `rclone`:

```bash
# Install rclone
sudo apt install rclone
# Configure rclone
rclone config
```

#### Create Services to Run Python Scripts
The two python scripts (_com_ports.py_ and _Measurement_synchronization.py_) are run as background services on Linux. This ensures they start automatically when the Raspberry Pi powers on. The service files are located in: _/etc/systemd/system_

- *read_ports.service* : Runs _com_ports.py_

```bash
[Unit]
Description=Measurement System (Serial)
After=network.target

[Service]
ExecStart=/home/fr1boise/Documents/Fungi/EVAL-AD590ELCZ_Pain/python_Code/.venv/bin/python3 /home/fr1boise/Documents/Fungi/EVAL-AD590ELCZ_Pain/python_Code/com_ports.py
WorkingDirectory=/home/fr1boise/Documents/Fungi/EVAL-AD590ELCZ_Pain/python_Code
Restart=always
RestartSec=5
User=fr1boise

PrivilegesStartOnly=true

[Install]
WantedBy=multi-user.target
```
- *measure_syncro.service* : Runs _Measurement_synchronization.py_

```bash
[Unit]
Description=Measurement System (Synchronisation)
After=network.target

[Service]
ExecStart=/home/fr1boise/Documents/Fungi/EVAL-AD590ELCZ_Pain/python_Code/.venv/bin/python3 /home/fr1boise/Documents/Fungi/EVAL-AD590ELCZ_Pain/python_Code/Measurement_synchronization.py
WorkingDirectory=/home/fr1boise/Documents/Fungi/EVAL-AD590ELCZ_Pain/python_Code
Restart=always
RestartSec=5
User=fr1boise

PrivilegesStartOnly=true

[Install]
WantedBy=multi-user.target
```

- Commands to enable and start a service:

```bash
sudo systemctl enable name.service
sudo systemctl start name.service
```
