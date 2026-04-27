import numpy as np
from random import *
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib import pyplot
from matplotlib.figure import Figure
from matplotlib.lines import Line2D  #CHANGED
from tkinter import Tk, Frame, Button #, IntVar
import socket

root = Tk()
root.wm_title("Vivent")  # Window name

NB_GRAPH = 8
SIMULATION_MODE = 0
TCP_IP = '160.98.87.167'
TCP_PORT = 50005
BUFFER_SIZE = 80  # Normally 1024, but we want fast response

class GraphWindow(Frame):
    
    '''def readData(self):
        """ This method reads a new set of data
        Parameters: none
        Return: array (librairie Numpy) containing recevied values
        """

        data = list()

        if SIMULATION_MODE == 1: # If in simulation mode, generate random data
            for i in range(0, NB_GRAPH):
                line_data = list()
                for i in range(0,100):
                    line_data.append(random())
                data.append(line_data)
        else # If in real mode, store received data
            # ANTHONY PUT YOUR SH*T HERE !
        return data'''
        
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
        self.figure.set_size_inches((10,9), forward=True) # dimensions of figure

        self.subplots = list() # will contain subplots objects
        self.lines = list() # will contain lines showed in subplots
        
        self.subplot1 = self.figure.add_subplot(111)  # create subplot 1

        for i in (range(0, NB_GRAPH)): # create empty lines
            #self.lines.append(self.subplot1.plot([], [])) #CHANGER
            self.lines.append(Line2D([],[])) #CHANGED
            self.subplot1.add_line(self.lines[i])#CHANGED
            
        self.subplot1.set_title("Voltage [V]")  # title of subplot 1
        self.subplot1.set_xlabel("1/20 of samples")
        self.subplot1.set_ylabel("y axis name")
        
        self.canvas = FigureCanvasTkAgg(self.figure, master=self.frame)  # create canvas
        self.canvas.get_tk_widget().grid(row=3,column=0,columnspan=3)

        self.button_quit = Button(self.frame,text="Quit") # create button "Quit"
        self.button_quit.grid(row=0, column=1)  # attach button "Quit" at position (0, 1)
        def quitButtonCallback(arg):
            """ Method called when button "Quit" is pressed. It quits
            program properly
            Parameters:
            - arg: This parameter is not used, but is necessary to avoid an
            error, because a button must be attached to a function with
            parameter
            Return: none
            """
            self.conn.close() # Close socket communication
            root.quit()     # Stops main loop
            root.destroy()  # Necessary with Windows to avoid error
                            # Fatal Python Error: PyEval_RestoreThread: NULL tstate
        self.button_quit.bind("<Button-1>", quitButtonCallback)

        ''' SOCKET server'''
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.s.bind((TCP_IP, TCP_PORT))
        self.s.listen(1)
        self.conn, self.addr = self.s.accept()
        print ('Connection address:', self.addr)

        self.temp = list()
        self.data_plot = list()
        self.data_plot = [[],[],[],[],[],[],[],[]]
        self.rest = 0
        self.cnt_data = 0
        self.data = list()
        self.data = [[],[]]
        self.last_i = 0
        self.ch = 0
        self.size = 0
        self.problem = 0
        
        self.frame.after(0, self.update_plots)
        
    def update_plots(self):
        """ Method called at regular interval of time. Performs a new measurement and plots result
        Parameters: none
        Return: none
        """
        #self.data = self.readData() # Read new data
        self.data[self.cnt_data] = self.conn.recv(BUFFER_SIZE)
        
        if not self.data[self.cnt_data]:
            self.conn.close()
            for i in range(0, 8):
                print("length of channel_", i, " = ", len(self.data_plot[i]))
            return

        if(self.rest == 1):
            self.rest = 0
            #print (self.last_i, "/", self.size, " ", self.data[1 - self.cnt_data][self.last_i:self.size], " ", self.data[1 - self.cnt_data][self.last_i:-1], " ", self.data[self.cnt_data][0:10])
            self.data[self.cnt_data] = self.data[1 - self.cnt_data][self.last_i:self.size] + self.data[self.cnt_data]
            #print (self.data[self.cnt_data])
            #print (" ")
        self.last_i = 0
        #print ("received data:", self.socket.data[cnt_data])
        for i in range(0,len(self.data[self.cnt_data])):
            if(self.data[self.cnt_data][i] == 32): # 32 = SPACE
                try:
                    self.data_plot[self.ch].append(int(self.data[self.cnt_data][self.last_i:i]))
                    self.ch += 1
                except:
                    print ("problem : last_i = ", self.last_i, " & i = ", i)
                    print (self.data[self.cnt_data])
                    self.problem = 1
                self.last_i = i + 1
                #self.ch += 1
                        
            if(self.data[self.cnt_data][i] == 0): # 0 = END OF STRING
                try:
                    self.data_plot[self.ch].append(int(self.data[self.cnt_data][self.last_i:i]))
                except:
                    print ("problem : last_i = ", self.last_i, " & i = ", i)
                    print (self.data[self.cnt_data])
                    self.problem = 1
                self.last_i = i + 1
                if(self.ch != 7 or self.problem == 1):
                    print("INCOMPLETE TRAME")
                    for j in range(0, (self.ch + 1)):
                        self.data_plot[j].pop()   
                self.ch = 0
                
        self.size = len(self.data[self.cnt_data])
        #print ("----------------------------------------", self.size)
        #print (self.data[self.cnt_data])
        #print ("last val = ", self.data[self.cnt_data][self.size-1], " ", self.data[self.cnt_data][0:self.size])
        if(self.data[self.cnt_data][self.size-1] == 32 or self.data[self.cnt_data][self.size-1] == 0):
            self.rest = 0
        else:
            self.rest = 1
        #print ("rest = ", self.rest)
            
        if(self.cnt_data == 0):
            self.cnt_data = 1
        else:
            self.cnt_data = 0
    
        # Plot
        for i in (range(0, NB_GRAPH)): # delete existing lines
            #self.subplot1.lines.pop() # CHANGER
            if(len(self.data_plot[i]) > 100):
                del self.data_plot[i][0:(len(self.data_plot[i]) - 100)]
                
        for i in (range(0, NB_GRAPH)): # Create new lines
            self.t = np.arange(0, len(self.data_plot[i]), 1) # Create time vector
            #self.lines[i] = self.subplot1.plot(self.t, self.data_plot[i]) # CHANGER
            self.lines[i].set_data(self.t, self.data_plot[i])

        self.subplot1.set_ylim([-2000, 500000]) # y limit 
        self.subplot1.set_xlim([0, 100]) # x limit

        self.canvas.draw()
        self.canvas.get_tk_widget().update_idletasks()

        self.frame.after(0, self.update_plots)    # call method update_plots again 
                                                # after 0 ms
        

# Main loop
GraphWindow().mainloop()  # Create an object GraphWindow and execute main loop
                          # Method mainloop() is inherited from class Frame


          
