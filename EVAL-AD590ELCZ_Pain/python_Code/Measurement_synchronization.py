import RPi.GPIO as GPIO
import time
import subprocess


GPIO.setmode(GPIO.BOARD)

led38 = 38
led40 = 40
led36 = 36
led37 = 37

GPIO.setup(led38, GPIO.OUT)
GPIO.setup(led40, GPIO.OUT)
GPIO.setup(led36, GPIO.OUT)
GPIO.setup(led37, GPIO.OUT)


def wifi_off():
    subprocess.run(["sudo", "rfkill", "block", "wifi"])

def wifi_on():
    subprocess.run(["sudo", "rfkill", "unblock", "wifi"])


while True:

    wifi_off()
    time.sleep(2)

    GPIO.output(led36, True)
    time.sleep(0.2)
    GPIO.output(led36, False)
    time.sleep(5)
    
    GPIO.output(led37, True)
    time.sleep(0.2)
    GPIO.output(led37, False)
    time.sleep(5)

    GPIO.output(led38, True)
    time.sleep(0.2)
    GPIO.output(led38, False)
    time.sleep(5)

    GPIO.output(led40, True)
    time.sleep(0.2)
    GPIO.output(led40, False)
    
    wifi_on()
    time.sleep(300)
