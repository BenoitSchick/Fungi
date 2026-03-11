import threading
import sys
import glob
import serial
from time import sleep
from queue import Queue


serialPort = serial.Serial(port = "COM5", baudrate=230400,
                           bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE)
serialPort3 = serial.Serial(port = "COM3", baudrate=230400,
                           bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE)

serialString = ""                           # Used to hold data coming over UART

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

def serialRead(serialPort):
        print(serialPort)
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
            data = str(serialPort.readline())
            if len(data) > 0:
                tmp = serialPort.name + ' : ' + data
                msgQueue.put(tmp)
            sleep(0.5)
    
availablePorts = serial_ports()
while True:
    print(availablePorts)
    for port in availablePorts:
        serialRead(port)
    sleep(1)
        
        

serialPort.close()
