import numpy as np
from random import *
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib import pyplot
from matplotlib.figure import Figure
from matplotlib.lines import Line2D  
from tkinter import Tk, Frame, Button, Label  #, IntVar
import socket
import time

root = Tk()
root.wm_title("Vivent")  # Window name

NB_CH = 9
SIMULATION_MODE = 0
TCP_IP = '160.98.87.167'
TCP_PORT = 50005
BUFFER_SIZE = 4000  # Normally 1024, but we want fast response
SAMPLE_TO_SHOW = 200
    
class GraphWindow(Frame):          
    def __init__(self):
        """ Constructor of class GraphWindow
        Parameters: none
        Return: none
        """

        """
        Structure of window:
        Window
            self.frame
                self.canvas (made from self.figure which contains plots)
                self.button_quit
        """
        
        Frame.__init__(self)
        self.frame = Frame()  # create a container (TKinter) for other widgets
        self.frame.grid() # attach frame to window
        self.frame.update_idletasks()
        self.figure = pyplot.figure()  # create figure (matplotlib)
        self.figure.set_size_inches((17,9), forward=True) # dimensions of figure

        self.subplots = list() # will contain subplots objects
        self.lines = list() # will contain lines showed in subplots
        
        self.subplot1 = self.figure.add_subplot(111)  # create subplot 1

        for i in (range(0, NB_CH)): # create empty lines
            self.lines.append(Line2D([],[])) 
            self.subplot1.add_line(self.lines[i])

        self.lines[0].set_color("blue")
        self.lines[1].set_color("magenta")
        self.lines[2].set_color("black")
        self.lines[3].set_color("orange")
        self.lines[4].set_color("purple")
        self.lines[5].set_color("green")
        self.lines[6].set_color("cyan")
        self.lines[7].set_color("gold")
        self.lines[8].set_color("red");

        self.subplot1.legend(('Ch. 0','Ch. 1','Ch. 2','Ch. 3','Ch. 4','Ch. 5','Ch. 6','Ch. 7', 'Error'), loc='upper left', shadow='true')
        self.subplot1.grid(True)
        
        self.subplot1.set_title("Vivent 8 channels sampling")  # title of subplot 1
        self.subplot1.set_xlabel("1/20 of samples")
        self.subplot1.set_ylabel("Voltage [mV]")
        
        self.canvas = FigureCanvasTkAgg(self.figure, master=self.frame)  # create canvas
        self.canvas.get_tk_widget().grid(row=1,column=0,columnspan=5)

        self.button_quit = Button(self.frame,text="Quit") # create button "Quit"
        self.button_quit.grid(row=0, column=0)  # attach button "Quit" at position (0, 1)
        self.button_connect = Button(self.frame,text="Connect") 
        self.button_connect.grid(row=0, column=1)
        self.button_disconnect = Button(self.frame,text="Disconnect") 
        self.button_disconnect.grid(row=0, column=2) 
        self.label_connecting = Label(self.frame, text="Disconnected", bg='red')
        self.label_connecting.grid(row=0, column=3)
        self.label_max_min = Label(self.frame, text="Min =        |       Max = ")
        self.label_max_min.grid(row=0, column=4)
        def quitButtonCallback(arg):
            """ Method called when button "Quit" is pressed. It quits
            program properly
            Parameters:
            - arg: This parameter is not used, but is necessary to avoid an
            error, because a button must be attached to a function with
            parameter
            Return: none
            """
            try:
                self.conn.shutdown(socket.SHUT_RDWR)
                self.conn.close() # Close socket communication
            except:
                pass
            root.quit()     # Stops main loop
            root.destroy()  # Necessary with Windows to avoid error
                            # Fatal Python Error: PyEval_RestoreThread: NULL tstate
        def connectButtonCallback(arg):
            if (self.connected == 1):
                return
            ''' SOCKET server'''
            self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.s.bind((TCP_IP, TCP_PORT))
            self.s.listen(0)
            print ("wait on client")
            self.label_connecting.config(text=" Connecting ", bg='cyan')
            self.label_connecting.update_idletasks()

            try:
                self.s.setblocking(0)
            except socket.error:
                print ('Error for setblocking(0)')
            else:
                pass
            connect_count = 0

            while(connect_count < 20):
                try:
                    self.conn, self.addr = self.s.accept()
                except socket.error:
                    connect_count += 1
                    time.sleep(1)
                else:
                    print ('Connection address:', self.addr)
                    connect_count = 100
                    self.label_connecting.config(text="Connected", bg='green')
                    self.s.setblocking(1)
            
            if(connect_count == 20):
                    self.label_connecting.config(text="Disconnected", bg='red')
                    print ('No client')
            else:  
                self.temp = list()
                self.data_plot = list()
                self.data_plot = [[],[],[],[],[],[],[],[],[]]
                self.rest = 0
                self.cnt_data = 0
                self.data = list()
                self.data = [[],[]]
                self.last_i = 0
                self.ch = 0
                self.size = 0
                self.problem = 0
                self.connected = 1
                self.frame.after(100, self.update_plots)
            return "break"

        def disconnectButtonCallback(arg):
            try:
                self.conn.shutdown(socket.SHUT_RDWR)
                self.conn.close() # Close socket communication
            except:
                pass
            self.label_connecting.config(text="Disconnected", bg='red')
            self.connected = 0
            
            return "break"

        self.button_quit.bind("<Button-1>", quitButtonCallback)
        self.button_connect.bind("<Button-1>", connectButtonCallback)
        self.button_disconnect.bind("<Button-1>", disconnectButtonCallback)
        self.connected = 0
        #self.frame.after(1, self.connect)
        
        
    def update_plots(self):
        """ Method called at regular interval of time. Performs a new measurement and plots result
        Parameters: none
        Return: none
        """
        if (self.connected == 0):
            return
        list_max = list()
        list_min = list()
        
        self.data[self.cnt_data] = self.conn.recv(BUFFER_SIZE)
        
        if not self.data[self.cnt_data]:
            self.conn.close()
            self.label_connecting.config(text="Disconnected", bg='red')
            self.connected = 0
            for i in range(0, NB_CH):
                print("length of channel_", i, " = ", len(self.data_plot[i]))
            return

        if(self.rest == 1):
            self.rest = 0
            self.data[self.cnt_data] = self.data[1 - self.cnt_data][self.last_i:self.size] + self.data[self.cnt_data]
        self.last_i = 0
        
        for i in range(0,len(self.data[self.cnt_data])):
            if(self.data[self.cnt_data][i] == 32): # 32 = SPACE
                try:
                    self.data_plot[self.ch].append((int(self.data[self.cnt_data][self.last_i:i])*0.000298))
                    self.ch += 1
                except:
                    print ("problem : last_i = ", self.last_i, " & i = ", i)
                    print (self.data[self.cnt_data])
                    self.problem = 1
                self.last_i = i + 1
                #self.ch += 1
                        
            if(self.data[self.cnt_data][i] == 0): # 0 = END OF STRING
                try:
                    self.data_plot[self.ch].append((int(self.data[self.cnt_data][self.last_i:i])*0.000298))
                except:
                    print ("problem : last_i = ", self.last_i, " & i = ", i)
                    print (self.data[self.cnt_data])
                    self.problem = 1
                self.last_i = i + 1
                if(self.ch != (NB_CH - 1) or self.problem == 1):
                    print("INCOMPLETE TRAME")
                    for j in range(0, (self.ch + 1)):
                        self.data_plot[j].pop()   
                self.ch = 0
                
        self.size = len(self.data[self.cnt_data])
        if(self.data[self.cnt_data][self.size-1] == 32 or self.data[self.cnt_data][self.size-1] == 0):
            self.rest = 0
        else:
            self.rest = 1
            
        if(self.cnt_data == 0):
            self.cnt_data = 1
        else:
            self.cnt_data = 0
    
        # Plot   
        for i in (range(0, NB_CH)): # Update lines
##            if(len(self.data_plot[i]) > SAMPLE_TO_SHOW):       # Create time vector
##                del self.data_plot[i][0:(len(self.data_plot[i]) - SAMPLE_TO_SHOW)]
            self.t = np.arange(0, len(self.data_plot[i]), 1)
            self.lines[i].set_data(self.t, self.data_plot[i]) # Update lines
        for i in (range(0, (NB_CH-1))): # axis limit without error line
            list_max.append(max(self.data_plot[i])) # Value maximum for axis limit
            list_min.append(min(self.data_plot[i])) # Value minimum for axis limit
#        list_max.append(max(self.data_plot[7])) # Value maximum for axis limit
#        list_min.append(min(self.data_plot[7])) # Value minimum for axis limit

        if(max(list_max) == min(list_min)):
            max_temp = max(max(list_max) * 1.1, max(list_max) * 0.9)
            min_temp = min(max(list_max) * 1.1, max(list_max) * 0.9)
        else:
            max_temp = 1.1 * max(list_max) - 0.1 * min(list_min)
            min_temp = 1.1 * min(list_min) - 0.1 * max(list_max)

        #self.label_max_min.config(text = "Min = " + str(min(list_min)) + "       |       Max = " + str(max(list_max)))
        self.label_max_min.config(text = "Min = " + str(min(list_min)) + " | Max = " + str(max(list_max)) + " | Med = " + str((sum(list_max)/len(list_max) + sum(list_min)/len(list_min))/2))
        self.subplot1.set_ylim([min_temp, max_temp]) # y limit
        self.subplot1.set_xlim([0, len(self.data_plot[0])]) # x limit
##        self.subplot1.set_xlim([0, SAMPLE_TO_SHOW]) # x limit

        self.canvas.draw()
        self.canvas.get_tk_widget().update_idletasks()

        self.frame.after(1, self.update_plots)    # call method update_plots again 
                                                  # after 1 ms
        

# Main loop
GraphWindow().mainloop()  # Create an object GraphWindow and execute main loop
                          # Method mainloop() is inherited from class Frame


          
