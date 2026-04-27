from tkinter import filedialog
from tkinter import messagebox
from tkinter import *

## for COM port
import sys
import glob

import serial
from serial.tools.list_ports import comports

Nstep=20
Vmin=0
Vmax=170
Vstep=10
barheight=120
button_width=6
sliderl=10
sliderw=20
windowtitle="MicroPerf GUI"

COMport_enable = 0
ser = serial.Serial()
destroy_selfCP = 0

self = Tk()
self.title(windowtitle)

def openfile():
    file_path=filedialog.askopenfilename()
    if len(file_path) != 0:
        ff=open(file_path,'r')
        line=ff.readline()
        cols=line.split()
        for x in range(Nstep):
            out1[x].set(str(cols[x]))

        line=ff.readline()
        cols=line.split()
        for x in range(Nstep):
            out2[x].set(str(cols[x]))

        line=ff.readline()
        cols=line.split()
        for x in range(Nstep):
            out3[x].set(str(cols[x]))

        line=ff.readline()
        cols=line.split()
        for x in range(Nstep):
            out4[x].set(str(cols[x]))

        line=ff.readline()
        cols=line.split()
        for x in range(Nstep):
            out5[x].set(str(cols[x]))
        ff.close()

def savefile():
    file_path=filedialog.asksaveasfilename()
    if len(file_path) != 0:
        ff=open(file_path,'w')
        for x in range(Nstep-1):
            ff.write(str(out1[x].get())+"\t")
        ff.write(str(out1[Nstep-1].get())+"\n")
        
        for x in range(Nstep-1):
            ff.write(str(out2[x].get())+"\t")
        ff.write(str(out2[Nstep-1].get())+"\n")
        
        for x in range(Nstep-1):
            ff.write(str(out3[x].get())+"\t")
        ff.write(str(out3[Nstep-1].get())+"\n")
        
        for x in range(Nstep-1):
            ff.write(str(out4[x].get())+"\t")
        ff.write(str(out4[Nstep-1].get())+"\n")
        
        for x in range(Nstep-1):
            ff.write(str(out5[x].get())+"\t")
        ff.write(str(out5[Nstep-1].get())+"\n")
        ff.close()
    
def openCOMport(port):
    print(port)
    ser.port = port 
    ser.baudrate=460800 #115200
    ser.parity=serial.PARITY_NONE
    ser.stopbits=serial.STOPBITS_ONE
    ser.bytesize=serial.EIGHTBITS
    ser.xonxoff = False     #disable software flow control
    ser.rtscts = True     #disable hardware (RTS/CTS) flow control
    ser.dsrdtr = False       #disable hardware (DSR/DTR) flow control
    ser.open()
    global COMport_enable
    COMport_enable = 1
    #self.subMenu.add_command(label='Liberate {} port'.format(port),command=liberate_COMport)
    
    selfCP.destroy()

def portIsUsable(portName):
    try:
       s = serial.Serial(port=portName)
       return True
    except serial.serialutil.SerialException:
       return False
    
def selectCOMport():
    windowtitle_CP = "COM port selection"
    global selfCP
    selfCP = Tk()
    selfCP.title(windowtitle_CP)
    my_row = 1
    labelinfo=Label(selfCP,text="--- Available ports:",font="bold")
    labelinfo.grid(row=my_row,column=1,padx=10,pady=10)
    my_row += 1

    lcomport = []
    bcomport = []
    m = 0
    
    for n, (port, desc, hwid) in enumerate(sorted(comports()), 1):
        if (portIsUsable(port) and (desc == 'JLink CDC UART Port ({})'.format(port))):
            label_CP=Label(selfCP,text=('--- {:2}: {}\n'.format((m + 1), desc)),font="bold")
            lcomport.append(label_CP)
            lcomport[m].grid(row=(my_row + m), column=1, padx=10, pady=10)

            button_CP = Button(selfCP, text=port, command=lambda port = port : openCOMport(port), width=button_width)
            """ the explication for this button creation is on the website:
                http://tkinter.unpythonic.net/wiki/CallbackConfusion
                under the section "Creating Buttons or Menu items in a loop:"
                So much tricky !!!!!!"""
            bcomport.append(button_CP) 
            bcomport[m].grid(row=(my_row + m), column=2, padx=10, pady=0)
            m += 1
            

def send_seq():
    global COMport_enable
    if COMport_enable == 1:
        print('\n')
        ser.write((int(0xff)).to_bytes(1, byteorder='big'))
        for x in range(Nstep):
            ser.write((int(out1[x].get()/10)).to_bytes(1, byteorder='big'))
            print((int(out1[x].get()/10)).to_bytes(1, byteorder='big'))
            
        print('\n')

        for x in range(Nstep):
            ser.write((int(out2[x].get()/10)).to_bytes(1, byteorder='big'))
            print((int(out2[x].get()/10)).to_bytes(1, byteorder='big'))

        print('\n')

        for x in range(Nstep):
            ser.write((int(out3[x].get()/10)).to_bytes(1, byteorder='big'))
            print((int(out3[x].get()/10)).to_bytes(1, byteorder='big'))

        print('\n')
 
        for x in range(Nstep):
            ser.write((int(out4[x].get()/10)).to_bytes(1, byteorder='big'))
            print((int(out4[x].get()/10)).to_bytes(1, byteorder='big'))

        print('\n')

        for x in range(Nstep):
            ser.write((int(out5[x].get()/10)).to_bytes(1, byteorder='big'))
            print((int(out5[x].get()/10)).to_bytes(1, byteorder='big'))

def exit_prog():
    global COMport_enable
    if messagebox.askokcancel("Quit", "Do you want to quit?"):
        if COMport_enable == 1:
            ser.close()
        global self
        self.destroy()

def liberate_COMport():
    global COMport_enable
    if COMport_enable == 1:
        #self.subMenu.remove_command(label='Liberate {} port'.format(port),command=liberate_COMport)
        ser.close()
    
# on window closing event (user clicking the 'X' button)
self.protocol("WM_DELETE_WINDOW", exit_prog)

top = self.winfo_toplevel()
self.menuBar = Menu(top)
top['menu'] = self.menuBar
self.subMenu = Menu(self.menuBar)
self.menuBar.add_cascade(label='Menu', menu=self.subMenu)
self.subMenu.add_command(label='Load sequence...',command=openfile)
self.subMenu.add_command(label='Save sequence...',command=savefile)
self.subMenu.add_command(label='Select COM port...',command=selectCOMport)
self.subMenu.add_command(label='SEND sequence',command=send_seq)
self.subMenu.add_command(label='EXIT',command=exit_prog)

#################### START
#def start_seq():
#bstart=Button(self,text="START",command=liberate_COMport,width=button_width) 
#bstart.grid(row=6,column=8,padx=10,pady=0)

#################### STOP
#def stop_seq():
#bstop=Button(self,text="STOP",command=liberate_COMport,width=button_width) 
#bstop.grid(row=6,column=12,padx=10,pady=0)

#################### OUT1
label1=Label(self,text="#1",font="bold")
label1.grid(row=1,column=1,padx=10,pady=0)
out1=list()
for i in range(Nstep):
    w=Scale(self,from_=Vmax,to=Vmin,resolution=Vstep,activebackground="green",cursor="sb_v_double_arrow",length=barheight,sliderlength=sliderl,width=sliderw)
    w.grid(row=1,column=(i+2),padx=0,pady=0)
    out1.append(w)

def reset_seq1():
    for i in range(Nstep):
        out1[i].set(Vmin)
breset1=Button(self,text="RESET",command=reset_seq1,width=button_width) 
breset1.grid(row=1,column=(Nstep+2),padx=10,pady=0)

##################### OUT2
label2=Label(self,text="#2",font="bold")
label2.grid(row=2,column=1,padx=10,pady=0)
out2=list()
for i in range(Nstep):
    w=Scale(self,from_=Vmax,to=Vmin,resolution=Vstep,activebackground="green",cursor="sb_v_double_arrow",length=barheight,sliderlength=sliderl,width=sliderw)
    w.grid(row=2,column=(i+2),padx=0,pady=0)
    out2.append(w)
    
def reset_seq2():
    for i in range(Nstep):
        out2[i].set(Vmin)
breset2=Button(self,text="RESET",command=reset_seq2,width=button_width) 
breset2.grid(row=2,column=(Nstep+2),padx=10,pady=0)

#################### OUT3
label3=Label(self,text="#3",font="bold")
label3.grid(row=3,column=1,padx=10,pady=0)
out3=list()
for i in range(Nstep):
    w=Scale(self,from_=Vmax,to=Vmin,resolution=Vstep,activebackground="green",cursor="sb_v_double_arrow",length=barheight,sliderlength=sliderl,width=sliderw)
    w.grid(row=3,column=(i+2),padx=0,pady=0)
    out3.append(w)

def reset_seq3():
    for i in range(Nstep):
        out3[i].set(Vmin)
breset3=Button(self,text="RESET",command=reset_seq3,width=button_width) 
breset3.grid(row=3,column=(Nstep+2),padx=10,pady=0)

##################### OUT4
label4=Label(self,text="#4",font="bold")
label4.grid(row=4,column=1,padx=10,pady=0)
out4=list()
for i in range(Nstep):
    w=Scale(self,from_=Vmax,to=Vmin,resolution=Vstep,activebackground="green",cursor="sb_v_double_arrow",length=barheight,sliderlength=sliderl,width=sliderw)
    w.grid(row=4,column=(i+2),padx=0,pady=0)
    out4.append(w)
    
def reset_seq4():
    for i in range(Nstep):
        out4[i].set(Vmin)
breset4=Button(self,text="RESET",command=reset_seq4,width=button_width) 
breset4.grid(row=4,column=(Nstep+2),padx=10,pady=0)
  
##################### OUT5
label5=Label(self,text="#5",font="bold")
label5.grid(row=5,column=1,padx=10,pady=0)
out5=list()
for i in range(Nstep):
    w=Scale(self,from_=Vmax,to=Vmin,resolution=Vstep,activebackground="green",cursor="sb_v_double_arrow",length=barheight,sliderlength=sliderl,width=sliderw)
    w.grid(row=5,column=(i+2),padx=0,pady=0)
    out5.append(w)
    
def reset_seq5():
    for i in range(Nstep):
        out5[i].set(Vmin)
breset5=Button(self,text="RESET",command=reset_seq5,width=button_width) 
breset5.grid(row=5,column=(Nstep+2),padx=10,pady=0)

mainloop()
