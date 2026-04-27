#!/usr/bin/env python
import socket
import time
TCP_IP = '160.98.87.167'
TCP_PORT = 50005
BUFFER_SIZE = 1024
MESSAGE = "Hello, World!\n"
MESSAGE_bytes = str.encode(MESSAGE)
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((TCP_IP, TCP_PORT))
time.sleep(1)
s.send(MESSAGE_bytes)
data = s.recv(BUFFER_SIZE)
s.close()
print ("received data:", data)
