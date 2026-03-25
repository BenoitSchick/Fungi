import RPi.GPIO as GPIO
import time

GPIO.setmode(GPIO.BOARD)

led38 = 38
led40 = 40
led36 = 36
led37 = 37

GPIO.setup(led38, GPIO.OUT)
GPIO.setup(led40, GPIO.OUT)
GPIO.setup(led36, GPIO.OUT)
GPIO.setup(led37, GPIO.OUT)


while True:

    #GPIO.output(led36, True)
    #time.sleep(0.2)
    #GPIO.output(led36, False)
    #time.sleep(5)
    
    #GPIO.output(led37, True)
    #time.sleep(0.2)
    #GPIO.output(led37, False)
    #time.sleep(5)

    GPIO.output(led38, True)
    time.sleep(0.2)
    GPIO.output(led38, False)
    time.sleep(5)

    #GPIO.output(led40, True)
    #time.sleep(0.2)
    #GPIO.output(led40, False)
    #time.sleep(5)



