import spidev
import time
import RPi.GPIO as GPIO

# =========================
# Configuration GPIO
# =========================

CS_PIN = 17   # GPIO17

GPIO.setmode(GPIO.BCM)
GPIO.setup(CS_PIN, GPIO.OUT)

GPIO.output(CS_PIN, 1)

# =========================
# Configuration SPI
# =========================

spi = spidev.SpiDev()

# bus 0, device 0 = /dev/spidev0.0
spi.open(0, 0)

spi.max_speed_hz = 10000   # 10 kHz
spi.mode = 0               # CPOL=0 CPHA=0
spi.bits_per_word = 8

print("=== SPI Scope Test Raspberry Pi ===")

while True:

    GPIO.output(CS_PIN, 0)

    spi.xfer2([0xAA])
    time.sleep(0.05)

    spi.xfer2([0x55])
    time.sleep(0.05)

    spi.xfer2([0xFF])
    time.sleep(0.05)

    spi.xfer2([0x00])
    time.sleep(0.05)

    GPIO.output(CS_PIN, 1)

    print("Trame envoyée")

    time.sleep(1)
