import numpy as np
import csv
import time
import visa
from random import *
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib import pyplot
from matplotlib.figure import Figure
from tkinter import Tk, Frame, Button, filedialog, Label, Entry
#import tkFileDialog

""" Main parameters """
SIMULATION_MODE = 1  # 1 for simulation, 0 for real measurement from impedancemeter
freq_start = 8E6    #9E6    # Start frequency
freq_stop = 12E6    #14E6    # Stop frequency
ampl_bottom = 700   #163   # Bottom display value
ampl_top = 7E3      #2E3      # Top display value
nb_points = 1001    # Number of points

root = Tk()
root.wm_title("Frequency response analyzer")  # Window name

class GraphWindow(Frame):
    
    def init_device(self):
        """ This method inits measurement
        Parameters: none
        Return: none
        """

        if SIMULATION_MODE == 0:
            rm = visa.ResourceManager()

            self.m4294a = rm.open_resource("GPIB2::2")
            self.m4294a.write("*IDN?")
            print('Ask device ID...')
            print(self.m4294a.read())
            # HEWLETT-PACKARD,4294A,MY43202956,REV1.11
            self.m4294a.write("*RST")
            self.m4294a.write("*CLS")

            self.m4294a.write("E4TP OFF")	# No adapter
            self.m4294a.write("PRES")	        # Reset to preset state
            self.m4294a.write("MEAS IMPH")	# Type of measurement (p.348)
            self.m4294a.write("SWPP FREQ")	# Set sweep parameter (p.449)
            self.m4294a.write("SWPT LIN")
            self.m4294a.write("STAR " + str(self.freq_start))  # Start frequency
            self.m4294a.write("STOP " + str(self.freq_stop))   # Stop frequency
            self.m4294a.write("POIN " + str(nb_points))	# Number of points
            self.m4294a.write("BWFACT 2")
            self.m4294a.write("POWMOD VOLT")    # OSC unit = volt
            self.m4294a.write("POWE 0.15V")     # OSC level
            self.m4294a.write("CONT")		# Continuous sweep
            self.m4294a.write("TRAC A")		    
            self.m4294a.write("BOTV " + str(self.ampl_bottom)) # Bottom display value
            self.m4294a.write("TOPV " + str(self.ampl_top))    # Top display value

            time.sleep(3) # Pause, necessary in order to let time to start
            
    def readData(self):
        """ This method reads a new set of data
        Parameters: none
        Return: array (librairie Numpy) containing recevied values
        """

        data = np.empty(0, dtype=float)
        if SIMULATION_MODE == 0:
            if (self.params_changed):
                self.m4294a.write("STAR " + str(self.freq_start))  # Set new start frequency
                self.m4294a.write("STOP " + str(self.freq_stop))  # Set new stop frequency 
                self.m4294a.write("BOTV " + str(self.ampl_bottom)) # Bottom display value
                self.m4294a.write("TOPV " + str(self.ampl_top)) # Top display value
                time.sleep(3) # Pause, necessary in order to let time to start
                self.params_changed = False
            self.m4294a.write("FORM2")   # Ask values in 32-bit floating point format

            values_temp = self.m4294a.query_binary_values("OUTPDTRC?", datatype='f', container=np.array, is_big_endian=True)
                        # raw received values. Contains for each point the real value and imaginary value
                        #(always 0 for impedance)
            for i in range(0, values_temp.size, 2):  # brows with increments of 2
                data = np.append(data, values_temp[i]) # get useful data from values_temp (1 value out of 2)

        else:
            if (self.params_changed):
                self.params_changed = False
                
            for i in range(0,100): # Generate fake data
                data = np.append(data, (self.ampl_top-self.ampl_bottom)*random()+self.ampl_bottom)

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

        # load parameters
        self.freq_start = freq_start
        self.freq_stop = freq_stop
        self.ampl_bottom = ampl_bottom
        self.ampl_top = ampl_top

        self.params_changed = False
        
        Frame.__init__(self)
        self.frame = Frame()  # create a container (TKinter) for other widgets
        self.frame.grid() # attach frame to window
        self.frame.update_idletasks()
        self.figure = pyplot.figure()  # create figure (matplotlib)
        self.figure.set_size_inches((10,6), forward=True) # dimensions of figure
        
        self.subplot1 = self.figure.add_subplot(111)  # create subplot 1
        self.line1, = self.subplot1.plot([], [])  # initialize line1 to plot
        self.subplot1.set_title("")  # title of subplot 1
        self.subplot1.set_xlabel("Frequency [MHz]")
        self.subplot1.set_ylabel("Impedance [Ohm]")

        # Elements of GUI
        self.canvas = FigureCanvasTkAgg(self.figure, master=self.frame)  # create canvas
        self.canvas.get_tk_widget().grid(row=1,column=0,columnspan=16)

        self.button_quit = Button(self.frame,text="Quit") # create button "Quit" inside frame
        self.button_quit.grid(row=0, column=15)  # attach button "Quit" at position (0, 3)

        self.button_measure = Button(self.frame,text="Measure") # create button "Measure" inside frame
        self.button_measure.grid(row=0, column=0)  # attach button "Measure" at position (0, 0)

        self.button_save = Button(self.frame,text="Save measurement") # create button "Save measurement" inside frame
        self.button_save.grid(row=0, column=3)  # attach button "Save measurement" at position (0, 1)

        self.label_startFreq = Label(self.frame, text="Start frequency:")
        self.label_startFreq.grid(row=2, column=0)

        self.entry_startFreq = Entry(self.frame)
        self.entry_startFreq.grid(row=2, column=1)
        self.entry_startFreq.insert(0, "{:1.3E}".format(self.freq_start))

        self.label_stopFreq = Label(self.frame, text="Stop frequency:")
        self.label_stopFreq.grid(row=3, column=0)

        self.entry_stopFreq = Entry(self.frame)
        self.entry_stopFreq.grid(row=3, column=1)
        self.entry_stopFreq.insert(0, "{:1.3E}".format(self.freq_stop))

        self.label_amplBottom = Label(self.frame, text="Amplitude bottom:")
        self.label_amplBottom.grid(row=4, column=0)

        self.entry_amplBottom = Entry(self.frame)
        self.entry_amplBottom.grid(row=4, column=1)
        self.entry_amplBottom.insert(0, "{:1.3E}".format(self.ampl_bottom))
        
        self.label_amplTop = Label(self.frame, text="Amplitude top:")
        self.label_amplTop.grid(row=5, column=0)

        self.entry_amplTop = Entry(self.frame)
        self.entry_amplTop.grid(row=5, column=1)
        self.entry_amplTop.insert(0, "{:1.3E}".format(self.ampl_top))

        
        
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

        def measureButtonCallback(arg):
            """ Method called when button "Measure" is pressed. It performs a measurement
            from impedancemeter.
            Parameters:
            - arg: This parameter is not used, but is necessary to avoid an
            error, because a button must be attached to a function with
            parameter
            Return: none
            """
            self.update_plots()
        self.button_measure.bind("<Button-1>", measureButtonCallback)

        def saveButtonCallback(arg):
            """ Method called when button "Save" is pressed. It saves measured data in a text file.
            Parameters:
            - arg: This parameter is not used, but is necessary to avoid an
            error, because a button must be attached to a function with
            parameter
            Return: none
            """

            fileName = filedialog.asksaveasfilename(title = "Select file to write",filetypes = (("CSV files","*.csv"),("all files","*.*")),
                                                    defaultextension=".csv")
            if not fileName: # check if dialog closed with "cancel"
                return "break" # allows to avoid that button stays pressed

            with open(fileName, 'w', newline='') as csvfile:
                spamwriter = csv.writer(csvfile, delimiter=',',
                                        quotechar='"', quoting=csv.QUOTE_MINIMAL)
                for i in range(0, self.data.size):
                    spamwriter.writerow([self.data[i]] + [self.freq[i]])  # Write a new row with impedance value and frequency
            
            print("File " + fileName + " saved.")
            return "break" # allows to avoid that button stays pressed
        self.button_save.bind("<Button-1>", saveButtonCallback)

        def setGenericParam(entry, param):
            """ Generic method to set a new val to a measurement parameter
            Parameters:
            - entry: reference to entry widget whose value has been modified
            Return: none
            - param: reference to parameter to be modified
            """
            temp_str = entry.get()
            print("Value of entry:" + entry.get())#DEBUG
            current_val = param
            try:
                param = float(entry.get())
            except:
                entry.delete(0, 'end')
                entry.insert(0, "{:1.3E}".format(param))
            else:
                if param != current_val: # if parameter has changed, mark it
                    self.params_changed = True

            return param
                    
        def setStartFreq(arg):
            """ Method to set min frequency
            Parameters:
            - arg: This parameter is not used, but is necessary to avoid an
            error, because a button must be attached to a function with
            parameter
            Return: none
            """
            self.freq_start = setGenericParam(self.entry_startFreq, self.freq_start)
            print(self.params_changed)    #DEBUG    
        self.entry_startFreq.bind("<Return>", setStartFreq)
        self.entry_startFreq.bind("<FocusOut>", setStartFreq)

        def setStopFreq(arg):
            """ Method to set max frequency
            Parameters:
            - arg: This parameter is not used, but is necessary to avoid an
            error, because a button must be attached to a function with
            parameter
            Return: none
            """
            self.freq_stop = setGenericParam(self.entry_stopFreq, self.freq_stop)
                    
        self.entry_stopFreq.bind("<Return>", setStopFreq)
        self.entry_stopFreq.bind("<FocusOut>", setStopFreq)

        def setAmplBottom(arg):
            """ Method to set bottom amplitude
            Parameters:
            - arg: This parameter is not used, but is necessary to avoid an
            error, because a button must be attached to a function with
            parameter
            Return: none
            """
            self.ampl_bottom = setGenericParam(self.entry_amplBottom, self.ampl_bottom)
                    
        self.entry_amplBottom.bind("<Return>", setAmplBottom)
        self.entry_amplBottom.bind("<FocusOut>", setAmplBottom)

        def setAmplTop(arg):
            """ Method to set top amplitude
            Parameters:
            - arg: This parameter is not used, but is necessary to avoid an
            error, because a button must be attached to a function with
            parameter
            Return: none
            """
            self.ampl_top = setGenericParam(self.entry_amplTop, self.ampl_top)
                    
        self.entry_amplTop.bind("<Return>", setAmplTop)
        self.entry_amplTop.bind("<FocusOut>", setAmplTop)
        
        self.init_device() # Initialize measurement device
        
        self.frame.after(100, self.update_plots) # Update plots after ... ms 
        
    def update_plots(self):
        """ Method called at regular interval of time. Performs a new measurement and plots result
        Parameters: none
        Return: none
        """
        self.data = self.readData() # Read new data
        self.freq = np.linspace(self.freq_start/1e6, self.freq_stop/1e6,num=self.data.size) # create frequency vector
        
        # Plot
        self.subplot1.set_xlim([float(self.freq_start)/1e6, float(self.freq_stop)/1e6]) # x limit of line 1
        self.subplot1.lines.remove(self.line1) # delete line 1
        self.line1, = self.subplot1.plot(self.freq, self.data, color="blue") # create line 1
        self.subplot1.set_ylim([self.ampl_bottom, self.ampl_top]) # y limit of line 1
    
        self.canvas.draw()
        self.canvas.get_tk_widget().update_idletasks()
        #self.frame.after(100, self.update_plots)    # call method update_plots again 
                                                    # after 100 ms

# Main loop
GraphWindow().mainloop()  # Create an object GraphWindow and execute main loop
                          # Method mainloop() is inherited from class Frame


          
