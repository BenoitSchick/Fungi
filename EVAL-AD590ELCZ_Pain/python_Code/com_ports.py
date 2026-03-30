import threading
import sys
import glob
import serial
import time
from queue import Queue
from pathlib import Path

# List of the threads
threads = []

# Define the message queue to send data from workers to main thread
msgQueue = Queue(maxsize=1000)

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
                                        baudrate=230400, 
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
                    #serialPort.reset_input_buffer()
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
        allCOMFileName = self._selectFileName(myMeasurementDir + 'AllCOM_Measurements')
        
        COM = {}
        file_handles = {}
        
        for port in availablePorts:
            safe_port = port.replace("/", "_")
            filename = self._selectFileName(myMeasurementDir + safe_port + '_Measurements')
            COM[port] = filename
            f = open(filename, "a")
            f.write('Freq [Hz],Magnitude [Ohm],Phase[Deg],Time of Measurement\n')
            file_handles[port] = f

        try:
            while True:
                newData = msgQueue.get()

                parts = newData.split(' : ', 1)
                if len(parts) < 2:
                    continue

                sender = parts[0].strip()
                value = parts[1].strip()
                timestamp = time.asctime(time.localtime(time.time()))
                line = f"{value},{timestamp}\n"

                # Trouve le bon fichier pour ce port
                matched_port = next((p for p in availablePorts if sender in p), None)
                if matched_port and matched_port in file_handles:
                    file_handles[matched_port].write(line)
                    if time.time() % 2 < 0.01:  # toutes 2 secondes
                        file_handles[matched_port].flush()
                else:
                    print(f"[WARN] Port inconnu : {sender}")

        finally:
            # Fermeture si le thread s'arrete
            for f in file_handles.values():
                f.close()
    
    def run(self):
        
        time.sleep(20) 

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
        # ports = glob.glob('/dev/tty[A-Za-z]*')
        ports = glob.glob('/dev/ttyACM*')
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
