import threading
import sys
import glob
import serial
import time
import RPi.GPIO as GPIO
import subprocess
from queue import Queue
from pathlib import Path


# Config GPIO
GPIO.setmode(GPIO.BOARD)

led38 = 38
led40 = 40
led36 = 36
led37 = 37

GPIO.setup(led38, GPIO.OUT)
GPIO.setup(led40, GPIO.OUT)
GPIO.setup(led36, GPIO.OUT)
GPIO.setup(led37, GPIO.OUT)


# Wifi control

def wifi_off():
    subprocess.run(["sudo", "rfkill", "block", "wifi"])

def wifi_on():
    subprocess.run(["sudo", "rfkill", "unblock", "wifi"])

# GPIO sequence
def gpio_control():
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




# List of the threads
threads = []

# Define the message queue to send data from workers to main thread
msgQueue = Queue()

# Time to wait between readings in second
waitingTime = 1

# Name of the directory where to store the measured data
myMeasurementDir = 'Measurement/'

firstWrite = True

class myThread(threading.Thread):
    def __init__(self, identifier):
        super(myThread, self).__init__()

    def serialRead(self, serialPort):
        try:
            serialPort = serial.Serial( port = serialPort,\
                                        baudrate=230400,\
                                        bytesize=8,\
                                        timeout=2,\
                                        stopbits=serial.STOPBITS_ONE)
            validPort = 1
        except serial.SerialException as e:
            if e.errno == 13:
                raise e
            validPort = 0
            pass
        except OSError:
            validPort = 0
            pass
        
        while (validPort):
            if(serialPort.in_waiting > 0):
                data = str(serialPort.readline())
                if len(data) > 10:
                    tmp = serialPort.name + ' : ' + data
                    # Debug 
                    #print(tmp)
                    msgQueue.put(tmp)
                    #time.sleep(waitingTime)
                    serialPort.reset_input_buffer()
            time.sleep(0.01)
    def _selectFileName(self, fileName):
        i = 0
        while True:
            path = fileName + '_' + str(i) + '.csv'
            myFile = Path(path)
            try:
                myPath = myFile.resolve(strict=True)
            except FileNotFoundError:
                # doesn't exist
                return path
            i=i+1
                

    def measurementDisplay(self, availablePorts):
        measurement = {}
        COM = []
        global firstWrite

        allCOMFileName = self._selectFileName(myMeasurementDir+'AllCOM_Measurements')
        index = 0
        for port in availablePorts:
            safe_port = port.replace("/", "_")
            COM.append(self._selectFileName(myMeasurementDir + safe_port + '_Measurements'))
            index+=1       

        while True:
            newData = msgQueue.get()
            
            sender = newData.split(' : ')[0]
            
            value = newData[newData.find('b')+2:newData.find('\n')-2]
            valueAllCOM = newData[:newData.find('\n')-2]
            measurement[sender] = value

            
            value = value + ',' + time.asctime(time.localtime(time.time()))
            valueAllCOM = valueAllCOM + ',' + time.asctime(time.localtime(time.time()))
            
           # file1 = open(allCOMFileName,"a")
           # file1.write(newData)
           # file1.write('\n')
           # file1.close()


            # i == index, elem == port
            # verify if sender=/dev/ttyACM0 is contained in the name of the port elem = /dev/ttyACM0
            indexAsList = [i for i, elem in enumerate(availablePorts) if sender in elem]
            
            
            indexAsStrings = [str(index) for index in indexAsList]
            indexAsString = "".join(indexAsStrings)
            indexAsInt = int(indexAsString)
            file2 = open(COM[indexAsInt],"a")
            
            if(firstWrite):
                file2.write('Freq [Hz],Magnitude [Ohm],Phase[Deg],Time of Measurement')
                file2.write('\n')
                firstWrite = False
            
            file2.write(value)
            file2.write('\n')
            file2.close()
    
    def run(self):

        availablePorts = serial_ports()

        print(availablePorts)

        for port in availablePorts:
            t = threading.Thread(target = self.serialRead, args=(port,))
            t.daemon = True
            threads.append(t)
            print("Thread for " + port + " created")

        # Thread for gathering all data
        t = threading.Thread(target = self.measurementDisplay, args=(availablePorts,))
        t.daemon = True
        threads.append(t)

        t_GPIO = threading.Thread(target = gpio_control)
        t_GPIO.daemon = True
        threads.append(t_GPIO)
        
        print("Thread for gathering data created")

        for thread in threads:
            thread.start()

# Source: https://stackoverflow.com/questions/12090503/listing-available-com-ports-with-python
def serial_ports():
    """ Lists serial port names

        :raises EnvironmentError:
            On unsupported or unknown platforms
        :returns:
            A list of the serial ports available on the system
    """
    if sys.platform.startswith('win'):
        ports = ['COM%s' % (i + 1) for i in range(256)]
    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this excludes your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')
    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/tty.*')
    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result

if __name__ == "__main__":
    my_thread = myThread(1)
    my_thread.run()

    while True:
        nop = 0
