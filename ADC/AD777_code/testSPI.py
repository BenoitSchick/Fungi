import spidev
import RPi.GPIO as GPIO
import time

CS = 17

GPIO.setmode(GPIO.BCM)
GPIO.setup(CS, GPIO.OUT)

GPIO.output(CS, 0)

spi = spidev.SpiDev()
spi.open(0, 0)

spi.mode = 0
spi.max_speed_hz = 10000

try:
    while True:

        GPIO.output(CS, 1)

        spi.xfer2([0xAA])
        time.sleep(0.05)

        spi.xfer2([0x55])
        time.sleep(0.05)

        GPIO.output(CS, 0)

        time.sleep(0.5)

finally:
    GPIO.output(CS, 0)
    GPIO.cleanup()
    spi.close()
