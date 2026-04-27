#!/usr/bin/env python

import socket


TCP_IP = '160.98.87.167'
TCP_PORT = 50005
BUFFER_SIZE = 80  # Normally 1024, but we want fast response

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((TCP_IP, TCP_PORT))
s.listen(1)
conn, addr = s.accept()
print ('Connection address:', addr)

temp = list()
data_plot = list()
data_plot = [[],[],[],[],[],[],[],[]]
first = 1
cnt_data = 0
data = list()
data = [[],[]]
last_i = 0
ch = 0
while 1:
    data[cnt_data] = conn.recv(BUFFER_SIZE)
    if not data[cnt_data]: break
    print ("received data:", data[cnt_data])
    for i in range(0,len(data[cnt_data])):
        if(data[cnt_data][i] == 32): # 32 = SPACE
            if(first == 1):
                data_plot[ch].append(int(data[cnt_data][last_i:i]))
                last_i = i + 1
                ch += 1
            else:
                #size_data = len(data[1 - cnt_data]) - 1
                if(data[1 - cnt_data][-1] == 0 or data[1 - cnt_data][-1] == 32):
                    data_plot[ch].append(int(data[cnt_data][last_i:i]))
                    last_i = i + 1
                    ch += 1
                else:
                    data_plot[ch].append(int(data[1 - cnt_data][last_i:BUFFER_SIZE - 1] + data[cnt_data][0:i]))
                    last_i = i + 1
                    ch += 1
                    
        if(data[cnt_data][i] == 0): # 0 = END OF STRING
            #size_data = len(data[1 - cnt_data]) - 1
            if(data[1 - cnt_data][-1] == 0 or data[1 - cnt_data][-1] == 32):
                data_plot[ch].append(int(data[cnt_data][last_i:i]))
                last_i = i + 1
                # if ch != 7 and ch = x<7 remove last data in ch0 to x
                ch = 0
            else:
                data_plot[ch].append(int(data[1 - cnt_data][last_i:BUFFER_SIZE - 1] + data[cnt_data][0:i]))
                last_i = i + 1
                # if ch != 7 and ch = x<7 remove last data in ch0 to x
                ch = 0
    first = 0
    if(cnt_data == 0): cnt_data = 1
    else: cnt_data = 0
    #conn.send(data)  # echo
for i in range(0, 8):
    print(data_plot[i])
conn.close()
