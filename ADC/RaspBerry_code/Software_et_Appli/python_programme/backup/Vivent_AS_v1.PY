import numpy as np
from random import *
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib import pyplot
from matplotlib.figure import Figure
from matplotlib.lines import Line2D  
from tkinter import Tk, Frame, Button #, IntVar
import socket

root = Tk()
root.wm_title("Vivent")  # Window name

NB_CH = 8
SIMULATION_MODE = 0
TCP_IP = '160.98.87.167'
TCP_PORT = 50005
BUFFER_SIZE = 4000  # Normally 1024, but we want fast response

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
        self.lines[1].set_color("red")
        self.lines[2].set_color("black")
        self.lines[3].set_color("orange")
        self.lines[4].set_color("purple")
        self.lines[5].set_color("green")
        self.lines[6].set_color("pink")
        self.lines[7].set_color("yellow")

        self.subplot1.legend(('Ch. 0','Ch. 1','Ch. 2','Ch. 3','Ch. 4','Ch. 5','Ch. 6','Ch. 7'), loc='upper left', shadow='true')
        
        self.subplot1.set_title("Vivent 8 channels sampling")  # title of subplot 1
        self.subplot1.set_xlabel("1/20 of samples")
        self.subplot1.set_ylabel("Voltage [uV]")
        
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
        print ("wait on client")
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
        list_max = list()
        list_min = list()
        
        self.data[self.cnt_data] = self.conn.recv(BUFFER_SIZE)
        
        if not self.data[self.cnt_data]:
            self.conn.close()
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
                    self.data_plot[self.ch].append((int(self.data[self.cnt_data][self.last_i:i])*0.3))
                    self.ch += 1
                except:
                    print ("problem : last_i = ", self.last_i, " & i = ", i)
                    print (self.data[self.cnt_data])
                    self.problem = 1
                self.last_i = i + 1
                #self.ch += 1
                        
            if(self.data[self.cnt_data][i] == 0): # 0 = END OF STRING
                try:
                    self.data_plot[self.ch].append((int(self.data[self.cnt_data][self.last_i:i])*0.3))
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
            if(len(self.data_plot[i]) > 200):       # Create time vector
                del self.data_plot[i][0:(len(self.data_plot[i]) - 200)]
            self.t = np.arange(0, len(self.data_plot[i]), 1)
            self.lines[i].set_data(self.t, self.data_plot[i]) # Update lines
            list_max.append(max(self.data_plot[i])) # Value maximum for axis limit
            list_min.append(min(self.data_plot[i])) # Value minimum for axis limit
        
        self.subplot1.set_ylim([min(list_min), max(list_max)]) # y limit 
        self.subplot1.set_xlim([0, 200]) # x limit

        self.canvas.draw()
        self.canvas.get_tk_widget().update_idletasks()

        self.frame.after(1, self.update_plots)    # call method update_plots again 
                                                  # after 0 ms
        

# Main loop
GraphWindow().mainloop()  # Create an object GraphWindow and execute main loop
                          # Method mainloop() is inherited from class Frame


          
