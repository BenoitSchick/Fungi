import numpy as np
from random import *
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib import pyplot
from matplotlib.figure import Figure
from matplotlib.lines import Line2D  #CHANGED
from tkinter import Tk, Frame, Button #, IntVar
import time

root = Tk()
root.wm_title("Vivent")  # Window name

NB_GRAPH = 8
SIMULATION_MODE = 1

class GraphWindow(Frame):
    
    def readData(self):
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
        #else: # If in real mode, store received data
            # ANTHONY PUT YOUR SH*T HERE !
        return data
        
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
            #self.lines.append(self.subplot1.plot([], [])) #CHANGED
            self.lines.append(Line2D([],[])) #CHANGED
            self.subplot1.add_line(self.lines[i])#CHANGED
            
        self.subplot1.set_title("Voltage [V]")  # title of subplot 1
        self.subplot1.set_xlabel("time [s]")
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
            root.quit()     # Stops main loop
            root.destroy()  # Necessary with Windows to avoid error
                            # Fatal Python Error: PyEval_RestoreThread: NULL tstate
        self.button_quit.bind("<Button-1>", quitButtonCallback)

        self.t = np.arange(0., 10.0, 0.1) # Create time vector
        
        self.frame.after(10, self.update_plots)
        
    def update_plots(self):
        """ Method called at regular interval of time. Performs a new measurement and plots result
        Parameters: none
        Return: none
        """
        self.data = self.readData() # Read new data
        
        # Plot
        '''for i in (range(0, NB_GRAPH)): # delete existing lines
            self.subplot1.lines.pop() # delete last element
            
        for i in (range(0, NB_GRAPH)): # Create new lines
            self.lines[i] = self.subplot1.plot(self.t, self.data[i]) # Create lines with new value
    '''
        
        for i in (range(0, NB_GRAPH)): # Create new lines
            self.lines[i].set_data(self.t, self.data[i])

            
        self.subplot1.set_ylim([0, 1]) # y limit of line 1

        self.canvas.draw()
        self.canvas.get_tk_widget().update_idletasks()
        self.frame.after(100, self.update_plots)    # call method update_plots again 
                                                    # after 100 ms

# Main loop
GraphWindow().mainloop()  # Create an object GraphWindow and execute main loop
                          # Method mainloop() is inherited from class Frame


          
